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
#include <minizip/unzip.h>

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

size_t aui::zlib::ZipEntry::Stream::read(char* dst, size_t size) {
    return unzReadCurrentFile(mHandle, dst, size);
}

aui::zlib::ZipEntry::Stream::~Stream() {
    if (mHandle) {
        unzCloseCurrentFile(mHandle);
    }
}

aui::zlib::ZipEntry::Stream aui::zlib::ZipEntry::open(const std::string& password) const {
    if (auto err = unzOpenCurrentFile3(mHandle, nullptr, nullptr, false, password.empty() ? nullptr : password.c_str()); err != UNZ_OK) {
        throw AZLibException("unzOpenCurrentFile3 failed: {}"_format(err));
    }
    return Stream(mHandle);
}

void aui::zlib::readZip(aui::no_escape<ISeekableInputStream> stream, const std::function<void(const ZipEntry&)>& visitor) {
    zlib_filefunc_def funcs = {
        .zopen_file = [](voidpf opaque, const char* filename, int mode) -> voidpf { return opaque; },
        .zread_file = [](voidpf opaque, voidpf stream, void* buf, uLong size) -> uLong {
            return static_cast<ISeekableInputStream*>(opaque)->read(static_cast<char*>(buf), size);
        },
        .zwrite_file = [](voidpf opaque, voidpf stream, const void* buf, uLong size) -> uLong { return 0; },
        .ztell_file = [](voidpf opaque, voidpf stream) -> long {
            return static_cast<ISeekableInputStream*>(opaque)->tell();
        },
        .zseek_file = [](voidpf opaque, voidpf stream, uLong offset, int origin) -> long {
            static_cast<ISeekableInputStream*>(opaque)->seek(offset, [&] {
                switch (origin) {
                    case ZLIB_FILEFUNC_SEEK_SET:
                    default:
                        return ISeekableInputStream::Seek::BEGIN;
                    case ZLIB_FILEFUNC_SEEK_CUR:
                        return ISeekableInputStream::Seek::CURRENT;
                    case ZLIB_FILEFUNC_SEEK_END:
                        return ISeekableInputStream::Seek::END;
                }
            }());
            return 0;
        },
        .zclose_file = [](voidpf opaque, voidpf stream) -> int { return 0; },
        .zerror_file = [](voidpf opaque, voidpf stream) -> int { return 0; },
        .opaque = stream.ptr(),
    };
    auto unzipHandle = unzOpen2("archive.zip", &funcs);

    if (unzipHandle == nullptr) {
        throw AZLibException("can't open ZipFileReader");
    }
    AUI_DEFER { unzClose(unzipHandle); };

    unz_global_info64 info;
    if (auto err = unzGetGlobalInfo64(unzipHandle, &info)) {
        throw AZLibException("unzGetGlobalInfo failed: {}"_format(err));
    }
    for (size_t i = info.number_entry; i > 0; i--) {
        char filename[0x400];

        unz_file_info64 fileInfo;
        if (auto err =
                unzGetCurrentFileInfo64(unzipHandle, &fileInfo, filename, sizeof(filename), nullptr, 0, nullptr, 0);
            err != UNZ_OK) {
            throw AZLibException("unzGetCurrentFileInfo64 failed: {}"_format(err));
        }

        ZipEntry ze;
        ze.name = filename;
        ze.mHandle = unzipHandle;
        visitor(ze);

        if (i > 1) {
            unzGoToNextFile(unzipHandle);
        }
    }
}

void aui::zlib::ExtractTo::operator()(const ZipEntry& zipEntry) const {
    AFileOutputStream(prefix / zipEntry.name) << zipEntry.open();
}
