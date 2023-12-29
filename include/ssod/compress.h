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
