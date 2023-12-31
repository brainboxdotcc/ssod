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
#pragma once
#include <dpp/dpp.h>
#include <string>

/**
 * @brief AES256 CBC encryption wrapper using OpenSSL EVP
 */
class aes256_cbc {
	/**
	 * @brief 16 character IV from config
	 */
	std::vector<uint8_t> iv;
public:
	/**
	 * @brief Construct a new aes256 cbc object
	 * 
	 * @param iv IV from config
	 */
	explicit aes256_cbc(std::vector<uint8_t> iv);
	/**
	 * @brief Encrypt using key
	 * 
	 * @param key key, must be exactly 32 bytes
	 * @param message message to encrypt
	 * @param output output message
	 */
	void encrypt(const std::vector<uint8_t>& key, const std::vector<uint8_t>& message, std::vector<uint8_t>& output) const;
	/**
	 * @brief Decrypt using key
	 * 
	 * @param key key, must be exactly 32 bytes
	 * @param message message to decrypt
	 * @param output output message
	 */
	void decrypt(const std::vector<uint8_t>& key, const std::vector<uint8_t>& message, std::vector<uint8_t>& output) const;
};

namespace security {
	/**
	 * @brief Initialise security wrapper
	 * 
	 * @param creator Creating D++ cluster
	 */
	void init(dpp::cluster& creator);
	/**
	 * @brief Encrypt a custom_id
	 * 
	 * @note A temporal key is generated for each call to this function and prepended to the ciphertext. The temporal key
	 * is only 32 bits in size and made of printable ascii, it cannot be any larger due to length restrictions of custome id
	 * fields. This is used to mutate the static key. Zlib compression is applied to the text before encryption, in an attempt
	 * to make it shorter. This also serves as a data integrity check, as the header will be invalid if the content does not
	 * decrypt correctly, causing an exception to be thrown when decrypting which causes an empty string to be returned by
	 * security::decrypt().
	 * 
	 * @param text custom_id value to encrypt. We can safely encrypt about 64 characters. Anything more won't fit, after base64 encoding
	 * @return std::string output encrypted content
	 */
	std::string encrypt(const std::string& text);
	/**
	 * @brief Decrypt a custom_id
	 * 
	 * @param text custom_id value to decrypt.
	 * @return std::string output decrypted content
	 */
	std::string decrypt(const std::string& text);
};