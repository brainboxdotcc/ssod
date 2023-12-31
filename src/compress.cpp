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
#include <ssod/compress.h>
#include <stdexcept>
#include <cstring>
#include <zlib.h>

std::string zlib::compress(const std::string& str, int compressionlevel) {
	z_stream zs;
	std::memset(&zs, 0, sizeof(zs));

	if (deflateInit(&zs, compressionlevel) != Z_OK) {
		throw std::runtime_error("deflateInit failed while compressing.");
	}

	zs.next_in = (Bytef*)str.data();
	zs.avail_in = str.size();

	int ret{};
	char outbuffer[10240];
	std::string outstring;

	do {
		zs.next_out = reinterpret_cast<Bytef*>(outbuffer);
		zs.avail_out = sizeof(outbuffer);
		ret = deflate(&zs, Z_FINISH);
		if (outstring.size() < zs.total_out) {
			outstring.append(outbuffer, zs.total_out - outstring.size());
		}
	} while (ret == Z_OK);

	deflateEnd(&zs);

	if (ret != Z_STREAM_END) {
		throw std::runtime_error(zs.msg);
	}

	return outstring;
}

std::string zlib::decompress(const std::string& str) {
	z_stream zs;
	std::memset(&zs, 0, sizeof(zs));

	if (inflateInit(&zs) != Z_OK) {
		throw(std::runtime_error("inflateInit failed while decompressing."));
	}

	zs.next_in = (Bytef*)str.data();
	zs.avail_in = str.size();

	int ret;
	char outbuffer[10240];
	std::string outstring;

	do {
		zs.next_out = reinterpret_cast<Bytef*>(outbuffer);
		zs.avail_out = sizeof(outbuffer);
		ret = inflate(&zs, 0);
		if (outstring.size() < zs.total_out) {
			outstring.append(outbuffer, zs.total_out - outstring.size());
		}
	} while (ret == Z_OK);

	inflateEnd(&zs);

	if (ret != Z_STREAM_END) {
		throw std::runtime_error(zs.msg);
	}

	return outstring;
}