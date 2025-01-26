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

#include "kAUI.h"

#include <stdexcept>
#include <string>
#include "AUI/Common/AByteBuffer.h"
#include "AUI/Traits/memory.h"

#include <zlib.h>

void aui::zlib::compress(AByteBufferView b, AByteBuffer& dst) {
    uLong len = b.size() * 3 / 2 + 0xff;
    dst.reserve(dst.getSize() + len);
    int r = compress2(
        reinterpret_cast<Bytef*>(const_cast<char*>(dst.end())), &len,
        reinterpret_cast<Bytef*>(const_cast<char*>(b.data())), b.size(), Z_BEST_COMPRESSION);
    if (r != Z_OK) {
        throw std::runtime_error(std::string("zlib compress error ") + std::to_string(r));
    }
    dst.setSize(dst.getSize() + len);
}

void aui::zlib::decompress(AByteBufferView b, AByteBuffer& dst) {
    for (size_t i = 4;; i++) {
        dst.reserve(b.size() * i);
        uLong len = dst.endReserved() - dst.end();
        int r = uncompress(
            reinterpret_cast<Bytef*>(dst.end()), &len, reinterpret_cast<Bytef*>(const_cast<char*>(b.data())), b.size());
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

_unique<IInputStream> aui::zlib::decompressToStream(AByteBufferView b) {
    class LZByteBufferViewDecompressStream : public IInputStream, public aui::noncopyable {
    public:
        LZByteBufferViewDecompressStream(AByteBufferView b) {
            aui::zero(mStream);
            mStream.next_in = (Bytef*) b.data();
            mStream.avail_out = 0;
            if (auto r = inflateInit(&mStream); r != Z_OK) {
                throw AZLibException("zlib decompress error " + AString::number(r));
            }
            mStream.avail_in = b.size();
        }

        size_t read(char* dst, size_t size) override {
            auto prevTotalOut = mStream.total_out;
            mStream.next_out = (Bytef*) dst;
            mStream.avail_out = size;
            auto r = inflate(&mStream, Z_NO_FLUSH);
            if (r != Z_STREAM_END && r != Z_OK) {
                throw AZLibException("zlib decompress error " + AString::number(r));
            }
            return mStream.total_out - prevTotalOut;
        }

        ~LZByteBufferViewDecompressStream() override { inflateEnd(&mStream); }

    private:
        z_stream mStream;
    };

    return std::make_unique<LZByteBufferViewDecompressStream>(b);
}
