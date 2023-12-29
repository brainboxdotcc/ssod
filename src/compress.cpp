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