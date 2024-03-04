/************************************************************************************
 * 
 * The Seven Spells Of Destruction
 *
 * Copyright 1993,2001,2023 Craig Edwards <brain@ssod.org>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ************************************************************************************/
#include <ssod/aes.h>
#include <ssod/config.h>
#include <dpp/dpp.h>
#include <ssod/game_dice.h>
#include <ssod/compress.h>
#include <vector>
#include <memory>

#include <openssl/aes.h>
#include <openssl/evp.h>

static const int B64index[256] = {
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  62, 63, 62, 62, 63,
	52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 0,  0,  0,  0,  0,  0,
	0,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14,
	15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 0,  0,  0,  0,  63,
	0,  26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
	41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51
};

#define DECL_OPENSSL_PTR(tname, free_func)	\
	struct openssl_##tname##_dtor {		\
		void operator()(tname* v) {	\
			free_func(v);		\
		}				\
	};					\
	typedef std::unique_ptr<tname, openssl_##tname##_dtor> tname##_t


DECL_OPENSSL_PTR(EVP_CIPHER_CTX, ::EVP_CIPHER_CTX_free);


aes256_cbc::aes256_cbc(std::vector<uint8_t> _iv) : iv(std::move(_iv)) {
}

void aes256_cbc::encrypt(const std::vector<uint8_t>& key, const std::vector<uint8_t>& message, std::vector<uint8_t>& output) const {
	output.resize(message.size() * AES_BLOCK_SIZE);
	int inlen = (int)message.size();
	int outlen = 0;
	size_t total_out = 0;

	EVP_CIPHER_CTX_t ctx(EVP_CIPHER_CTX_new());
	const std::vector<uint8_t> enc_key = key;

	EVP_EncryptInit(ctx.get(), EVP_aes_256_cbc(), enc_key.data(), iv.data());
	EVP_EncryptUpdate(ctx.get(), output.data(), &outlen, message.data(), inlen);
	total_out += outlen;
	EVP_EncryptFinal(ctx.get(), output.data() + total_out, &outlen);
	total_out += outlen;

	output.resize(total_out);
}

void aes256_cbc::decrypt(const std::vector<uint8_t>& key, const std::vector<uint8_t>& message, std::vector<uint8_t>& output) const {
	output.resize(message.size() * 3);
	int outlen = 0;
	size_t total_out = 0;

	EVP_CIPHER_CTX_t ctx(EVP_CIPHER_CTX_new());
	const std::vector<uint8_t> enc_key = key;
	std::vector<uint8_t> target_message;
	std::vector<uint8_t> _iv;

	_iv = iv;
	target_message = message;

	int inlen = (int)target_message.size();

	EVP_DecryptInit(ctx.get(), EVP_aes_256_cbc(), enc_key.data(), _iv.data());
	EVP_DecryptUpdate(ctx.get(), output.data(), &outlen, target_message.data(), inlen);
	total_out += outlen;
	EVP_DecryptFinal(ctx.get(), output.data()+outlen, &outlen);
	total_out += outlen;

	output.resize(total_out);
}

static std::vector<uint8_t> str_to_bytes(const std::string& message) {
	std::vector<uint8_t> out(message.size());
	for(size_t n = 0; n < message.size(); n++) {
		out[n] = message[n];
	}
	return out;
}

static std::string bytes_to_str(const std::vector<uint8_t>& bytes) {
	return std::string(bytes.begin(), bytes.end());
}

const std::string b64decode(const void* data, size_t len) {
	if (len == 0) return "";

	auto *p = (unsigned char*) data;
	size_t j = 0,
	pad1 = len % 4 || p[len - 1] == '=',
	pad2 = pad1 && (len % 4 > 2 || p[len - 2] != '=');
	const size_t last = (len - pad1) / 4 << 2;
	std::string result(last / 4 * 3 + pad1 + pad2, '\0');
	auto *str = (unsigned char*) &result[0];

	for (size_t i = 0; i < last; i += 4) {
		int n = B64index[p[i]] << 18 | B64index[p[i + 1]] << 12 | B64index[p[i + 2]] << 6 | B64index[p[i + 3]];
		str[j++] = n >> 16;
		str[j++] = n >> 8 & 0xFF;
		str[j++] = n & 0xFF;
	}
	if (pad1) {
		int n = B64index[p[last]] << 18 | B64index[p[last + 1]] << 12;
		str[j++] = n >> 16;
		if (pad2) {
			n |= B64index[p[last + 2]] << 6;
			str[j++] = n >> 8 & 0xFF;
		}
	}
	return result;
}

namespace security {
	aes256_cbc* enc = nullptr;
	dpp::cluster* bot = nullptr;

	std::string random_string(size_t len) {
		std::string out;
		for (size_t x = 0; x < len; ++x) {
			out += (char)random(65, 90);
		}
		return out;
	}

	void init(dpp::cluster& creator) {
		enc = new aes256_cbc(str_to_bytes(config::get("encryption")["iv"]));
		bot = &creator;
	}

	/* We can fit 3x32 byte AES128 blocks into a 100-char Discord custom_id, plus a 4 character temporal key */
	std::string encrypt(const std::string& text) {
		std::vector<uint8_t> enc_result;
		std::string key = config::get("encryption")["key"];
		std::string temporal_key = random_string(4);
		for (size_t x = 0; x != 4; ++x) {
			key[x * 4] ^= temporal_key[x];
		}
		enc->encrypt(str_to_bytes(key), str_to_bytes(zlib::compress(text)), enc_result);
		std::string r = temporal_key + dpp::base64_encode(enc_result.data(), enc_result.size());
		if (r.length() > 100) {
			bot->log(dpp::ll_error, "Encrypted component ID [" + text + "] is longer than 100 characters and will not fit in the ID field!");
		}
		return r;
	}

	std::string decrypt(const std::string& text) {
		std::string temporal_key{text.substr(0, 4)};
		std::string ciphertext{text.substr(4, text.length() - 4)};
		std::vector<uint8_t> dec_result;
		std::string key = config::get("encryption")["key"];
		for (size_t x = 0; x != 4; ++x) {
			key[x * 4] ^= temporal_key[x];
		}
		std::string decoded = b64decode(ciphertext.data(), ciphertext.length());
		enc->decrypt(str_to_bytes(key), str_to_bytes(decoded), dec_result);
		try {
			return zlib::decompress(bytes_to_str(dec_result));
		}
		catch (const std::exception&) {
			/* Decompression failure, invalid encrypted ID */
			return "";
		}
	}
};