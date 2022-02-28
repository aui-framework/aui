/**
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
 */

#include "LZ.h"

#include <stdexcept>
#include <string>
#include "AUI/Common/AByteBuffer.h"

#include <zlib.h>


void LZ::compress(AByteBufferView b, AByteBuffer& dst)
{
    uLong len = b.size() * 3 / 2 + 0xff;
	dst.reserve(dst.getSize() + len);
	int r = compress2(reinterpret_cast<Bytef*>(const_cast<char*>(dst.end())), &len,
                      reinterpret_cast<Bytef*>(const_cast<char*>(b.data())), b.size(), Z_BEST_COMPRESSION);
	if (r != Z_OK)
	{
		throw std::runtime_error(std::string("zlib compress error ") + std::to_string(r));
	}
	dst.setSize(dst.getSize() + len);
}

void LZ::decompress(AByteBufferView b, AByteBuffer& dst)
{
	for (size_t i = 4;; i++) {
		dst.reserve(b.size() * i);
        uLong len = dst.endReserved() - dst.end();
		int r = uncompress(reinterpret_cast<Bytef*>(dst.end()), &len,
                           reinterpret_cast<Bytef*>(const_cast<char*>(b.data())), b.size());
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