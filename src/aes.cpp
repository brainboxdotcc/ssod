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
#include <cstdint>
#include <ctime>
#include <openssl/hmac.h>
#include <openssl/crypto.h>
#include <openssl/sha.h>
#include <openssl/rand.h>
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
	output.clear();
	if (key.size() != 32 || iv.size() != AES_BLOCK_SIZE || message.empty() || message.size() % AES_BLOCK_SIZE != 0) {
		return;
	}
	output.resize(message.size() + AES_BLOCK_SIZE);
	int outlen = 0;
	size_t total_out = 0;

	EVP_CIPHER_CTX_t ctx(EVP_CIPHER_CTX_new());
	const std::vector<uint8_t> enc_key = key;
	std::vector<uint8_t> target_message;
	std::vector<uint8_t> _iv;

	_iv = iv;
	target_message = message;

	int inlen = (int)target_message.size();

	if (!ctx || EVP_DecryptInit_ex(ctx.get(), EVP_aes_256_cbc(), nullptr, enc_key.data(), _iv.data()) != 1 ||
		EVP_DecryptUpdate(ctx.get(), output.data(), &outlen, target_message.data(), inlen) != 1) {
		output.clear();
		return;
	}
	total_out += outlen;
	if (EVP_DecryptFinal_ex(ctx.get(), output.data() + total_out, &outlen) != 1) {
		output.clear();
		return;
	}
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
	std::unique_ptr<aes256_cbc> enc{};
	dpp::cluster* bot{nullptr};
	static constexpr uint32_t temporal_epoch = 1793491200;
	static constexpr uint32_t temporal_mask = 0xFFFFFF;
	static constexpr uint32_t temporal_lifetime = 86400;

	static std::string make_temporal_key() {
		const uint32_t timestamp = static_cast<uint32_t>(std::time(nullptr) - temporal_epoch) & temporal_mask;

		uint8_t random = 0;
		if (RAND_bytes(&random, sizeof(random)) != 1) {
			throw std::runtime_error("Unable to generate temporal key");
		}

		/*
		 * Eight nibbles, each containing three timestamp bits
		 * and one random bit.
		 *
		 * This gives 24 timestamp bits and 8 random bits.
		 */
		uint32_t value = 0;

		for (unsigned int i = 0; i < 8; ++i) {
			value |= ((timestamp >> (i * 3)) & 7u) << (i * 4 + 1);
			value |= ((static_cast<uint32_t>(random) >> i) & 1u) << (i * 4);
		}

		std::string result(4, '\0');

		for (unsigned int i = 0; i < 4; ++i) {
			result[i] = static_cast<char>((value >> (24 - i * 8)) & 0xFF);
		}

		return result;
	}

	static uint32_t temporal_timestamp(const std::string& temporal_key) {
		uint32_t value = 0;

		for (unsigned int i = 0; i < 4; ++i) {
			value = (value << 8) | static_cast<uint8_t>(temporal_key[i]);
		}

		uint32_t timestamp = 0;

		for (unsigned int i = 0; i < 8; ++i) {
			timestamp |= ((value >> (i * 4 + 1)) & 7u) << (i * 3);
		}

		return timestamp;
	}

	static bool valid_temporal_key(const std::string& temporal_key) {
		if (temporal_key.size() != 4) {
			return false;
		}

		const std::time_t now = std::time(nullptr);

		if (now < temporal_epoch) {
			return false;
		}

		const uint32_t current = static_cast<uint32_t>(now - temporal_epoch) & temporal_mask;
		const uint32_t timestamp = temporal_timestamp(temporal_key);
		const uint32_t age = (current - timestamp) & temporal_mask;

		return age <= temporal_lifetime;
	}

	std::string random_string(size_t len) {
		std::string out;
		for (size_t x = 0; x < len; ++x) {
			out += (char)d_random(65, 90);
		}
		return out;
	}

	void init(dpp::cluster& creator) {
		enc = std::make_unique<aes256_cbc>(str_to_bytes(config::get("encryption")["iv"]));
		bot = &creator;
	}

	/* We can fit 3x32 byte AES128 blocks into a 100-char Discord custom_id, plus a 4 character temporal key */
	std::string encrypt(const std::string& text) {
		std::vector<uint8_t> enc_result;
		std::string key = config::get("encryption")["key"];
		std::string temporal_key = make_temporal_key();

		for (size_t x = 0; x != 4; ++x) {
			key[x * 4] ^= temporal_key[x];
		}

		/*
		 * The temporal key is repeated inside the encrypted content.
		 * Decryption must confirm that the inner and outer values match.
		 */
		enc->encrypt(str_to_bytes(key), str_to_bytes(zlib::compress(temporal_key + text)), enc_result);

		/*
		 * Base64 encode the complete binary packet:
		 *
		 * [4-byte temporal key][AES ciphertext]
		 */
		const std::string packet = temporal_key + bytes_to_str(enc_result);
		const std::string r = dpp::base64_encode(reinterpret_cast<const uint8_t*>(packet.data()), packet.size());

		if (r.length() > 100) {
			bot->log(dpp::ll_error, "Encrypted component ID [" + text + "] is longer than 100 characters and will not fit in the ID field!");
		}

		return r;
	}

	std::string decrypt(const std::string& text) {
		/*
		 * Entire packet is Base64:
		 *
		 * [4-byte temporal key][AES ciphertext]
		 */
		if (text.size() < 28 || text.size() > 100 || text.size() % 4 != 0) {
			return "";
		}

		const size_t padding = (text.back() == '=') + (text.size() > 1 && text[text.size() - 2] == '=');

		for (size_t i = 0; i < text.size(); ++i) {
			const unsigned char c = static_cast<unsigned char>(text[i]);

			if (i >= text.size() - padding) {
				if (c != '=') {
					return "";
				}
			} else if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '+' || c == '/')) {
				return "";
			}
		}

		const size_t decoded_length = text.size() / 4 * 3 - padding;

		if (decoded_length < 4 + AES_BLOCK_SIZE || (decoded_length - 4) % AES_BLOCK_SIZE != 0) {
			return "";
		}

		const std::string packet = b64decode(text.data(), text.length());

		if (packet.size() != decoded_length) {
			return "";
		}

		const std::string temporal_key = packet.substr(0, 4);

		if (!valid_temporal_key(temporal_key)) {
			return "";
		}

		std::string key = config::get("encryption")["key"];

		if (key.size() != 32 || !enc) {
			return "";
		}

		for (size_t x = 0; x != 4; ++x) {
			key[x * 4] ^= temporal_key[x];
		}

		const std::string ciphertext = packet.substr(4);
		std::vector<uint8_t> dec_result;

		enc->decrypt(str_to_bytes(key), str_to_bytes(ciphertext), dec_result);

		if (dec_result.empty()) {
			return "";
		}

		try {
			const std::string plaintext = zlib::decompress(bytes_to_str(dec_result), 4096);

			/*
			 * The timestamp/random bytes inside the ciphertext
			 * must match the outer temporal key exactly.
			 */
			if (plaintext.size() < 4 || plaintext.compare(0, 4, temporal_key) != 0) {
				return "";
			}

			return plaintext.substr(4);
		} catch (const std::exception&) {
			return "";
		}
	}

static std::string hmac_key() {
		const std::string key = config::get("encryption")["key"];
		static constexpr char purpose[] = "ssod-app-custom-id-v1";

		unsigned char digest[EVP_MAX_MD_SIZE];
		unsigned int length = 0;

		if (!HMAC(EVP_sha256(), key.data(), static_cast<int>(key.size()), reinterpret_cast<const unsigned char*>(purpose), sizeof(purpose) - 1, digest, &length)) {
			return "";
		}

		return std::string(reinterpret_cast<const char*>(digest), length);
	}

	std::string sign(const std::string& id) {
		const std::string key = hmac_key();

		if (key.empty()) {
			return "";
		}

		unsigned char digest[EVP_MAX_MD_SIZE];
		unsigned int length = 0;

		if (!HMAC(EVP_sha256(), key.data(), static_cast<int>(key.size()), reinterpret_cast<const unsigned char*>(id.data()), id.size(), digest, &length)) {
			return "";
		}

		static constexpr char hex[] = "0123456789abcdef";

		std::string result = id + ".";
		result.reserve(id.size() + 1 + length * 2);

		for (unsigned int i = 0; i < length; ++i) {
			result += hex[digest[i] >> 4];
			result += hex[digest[i] & 15];
		}

		return result;
	}

	std::string verify_signed(const std::string& id) {
		const size_t separator = id.rfind('.');

		if (separator == std::string::npos || separator == 0 || id.size() - separator - 1 != SHA256_DIGEST_LENGTH * 2) {
			return "";
		}

		const std::string unsigned_id = id.substr(0, separator);
		const std::string expected = sign(unsigned_id);

		if (expected.empty() || expected.size() != id.size()) {
			return "";
		}

		if (CRYPTO_memcmp(expected.data(), id.data(), id.size()) != 0) {
			return "";
		}

		return unsigned_id;
	}
};