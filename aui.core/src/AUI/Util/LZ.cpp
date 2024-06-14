/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
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