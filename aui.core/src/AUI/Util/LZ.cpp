// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2023 Alex2772
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

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