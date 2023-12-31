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
#include <string>

/**
 * @brief Simple zlib wrapper for compressing and decompressing strings
 */
struct zlib {
	/**
	 * @brief Compress a string using zlib
	 * 
	 * @param str String to compress
	 * @param compressionlevel ZLib compression level, 0 through 9
	 * @return std::string Compressed string
	 */
	static std::string compress(const std::string& str, int compressionlevel = 9);
	/**
	 * @brief Decompress a string using zlib
	 * 
	 * @param str String to decompress
	 * @return std::string Decompressed string
	 */
	static std::string decompress(const std::string& str);
};
