#include "LZ.h"

#include <stdexcept>
#include <string>
#include "AUI/Common/ByteBuffer.h"

#include <zlib.h>


void LZ::compress(const ByteBuffer& b, ByteBuffer& dst)
{
	dst.reserve(dst.getSize() + b.getAvailable() + 0xff);
	uLong len = b.getAvailable() + 0xff;
	int r = compress2(reinterpret_cast<Bytef*>(const_cast<char*>(dst.getCurrentPosAddress())), &len,
		reinterpret_cast<Bytef*>(const_cast<char*>(b.getCurrentPosAddress())), b.getAvailable(), Z_BEST_COMPRESSION);
	if (r != Z_OK)
	{
		throw std::runtime_error(std::string("zlib compress error ") + std::to_string(r));
	}
	dst.setSize(dst.getSize() + len);
	dst.setCurrentPos(dst.getSize());
}

void LZ::decompress(const ByteBuffer& b, ByteBuffer& dst)
{
	for (size_t i = 4;; i++) {
		dst.reserve(b.getAvailable() * i);
		uLong len = dst.getReserved();
		int r = uncompress(reinterpret_cast<Bytef*>(const_cast<char*>(dst.getCurrentPosAddress())), &len,
			reinterpret_cast<Bytef*>(const_cast<char*>(b.getCurrentPosAddress())), b.getAvailable());
		switch (r) {
		case Z_BUF_ERROR:
			continue;
		case Z_OK:
			dst.setSize(dst.getSize() + len);
			return;
		default:
			throw AZLibException("zlib decompress error " + AString::number(r));
		}
	}
}