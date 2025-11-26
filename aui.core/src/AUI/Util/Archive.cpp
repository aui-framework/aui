// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2025 Alex2772 and Contributors
//
// SPDX-License-Identifier: MPL-2.0
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.


#include "Archive.h"

#include "LZ.h"
#include "kAUI.h"

#include <AUI/IO/AFileOutputStream.h>
#include <minizip/unzip.h>
#include <minizip/zip.h>


void aui::archive::zip::read(aui::no_escape<ISeekableInputStream> stream, const std::function<void(const FileEntry&)>& visitor) {
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
                        return ASeekDir::BEGIN;
                    case ZLIB_FILEFUNC_SEEK_CUR:
                        return ASeekDir::CURRENT;
                    case ZLIB_FILEFUNC_SEEK_END:
                        return ASeekDir::END;
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

    ArchiveInfo archiveInfo {
        .numberOfFiles = static_cast<std::size_t>(info.number_entry),
    };

    for (size_t i = info.number_entry; i > 0; i--) {
        char filename[0x400];

        unz_file_info64 fileInfo;
        if (auto err =
                unzGetCurrentFileInfo64(unzipHandle, &fileInfo, filename, sizeof(filename), nullptr, 0, nullptr, 0);
            err != UNZ_OK) {
            throw AZLibException("unzGetCurrentFileInfo64 failed: {}"_format(err));
        }

        class ZipFileEntry: public FileEntry, public IInputStream {
        public:
            explicit ZipFileEntry(unzFile handle, ArchiveInfo& archiveInfo) : FileEntry(archiveInfo), mHandle(handle) {}

            ~ZipFileEntry() override {
                if (mFileOpened) {
                    unzCloseCurrentFile(mHandle);
                }
            }
            size_t read(char* dst, size_t size) override {
                AUI_ASSERT(mFileOpened);
                return unzReadCurrentFile(mHandle, dst, size);
            }

            no_escape<IInputStream> open(const std::string& password) const override {
                AUI_ASSERT(mFileOpened == false);
                if (auto err = unzOpenCurrentFile3(mHandle, nullptr, nullptr, false, password.empty() ? nullptr : password.c_str()); err != UNZ_OK) {
                    throw AZLibException("unzOpenCurrentFile3 failed: {}"_format(err));
                }
                mFileOpened = true;
                return const_cast<ZipFileEntry*>(this);
            }
        private:
            unzFile mHandle;
            mutable bool mFileOpened = false;
        } ze(unzipHandle, archiveInfo);
        ze.name = filename;

        visitor(ze);

        if (i > 1) {
            unzGoToNextFile(unzipHandle);
        }
    }
}

void aui::archive::ExtractTo::operator()(const FileEntry& zipEntry) const {
    if (zipEntry.name.ends_with('/')) {
        // directory?
        return;
    }
    APath dst = prefix / pathProjection(APath(zipEntry.name));
    dst.parent().makeDirs();
    AFileOutputStream(dst) << *zipEntry.open();
    dst.chmod(0755);
}

// Writer implementation
namespace aui::archive::zip {

Writer::Writer(_unique<ISeekableOutputStream> to): mZipFile(std::move(to)) {
    zlib_filefunc_def funcs = {
        .zopen_file = [](voidpf opaque, const char* filename, int mode) -> voidpf { return opaque; },
        .zread_file =  nullptr,
        .zwrite_file = [](voidpf opaque, voidpf stream, const void* buf, uLong size) -> uLong {
            static_cast<ISeekableOutputStream*>(opaque)->write(static_cast<const char*>(buf), size);
            return size;
        },
        .ztell_file = [](voidpf opaque, voidpf stream) -> long {
            return static_cast<ISeekableOutputStream*>(opaque)->tell();
        },
        .zseek_file = [](voidpf opaque, voidpf stream, uLong offset, int origin) -> long {
            static_cast<ISeekableOutputStream*>(opaque)->seek(offset, [&] {
                switch (origin) {
                    default:
                    case ZLIB_FILEFUNC_SEEK_SET: return ASeekDir::BEGIN;
                    case ZLIB_FILEFUNC_SEEK_CUR: return ASeekDir::CURRENT;
                    case ZLIB_FILEFUNC_SEEK_END: return ASeekDir::END;
                }
            }());
            return 0;
        },
        .zclose_file = [](voidpf opaque, voidpf stream) -> int { return 0; },
        .zerror_file = [](voidpf opaque, voidpf stream) -> int { return 0; },
        .opaque = mZipFile.get(),
    };
    mHandle = zipOpen2("archive.zip", APPEND_STATUS_CREATE, nullptr, &funcs);
    if (!mHandle) {
        throw AZLibException("can't open aui::archivw::zip::Writer zipOpen2 failed");
    }
}

Writer::~Writer() {
    zipClose(mHandle, nullptr);
}

void Writer::openFileInZip(const APath& filename, const std::function<void(IOutputStream&)>& visitor) const {
    zip_fileinfo zi{};
    if (auto code = zipOpenNewFileInZip(
            mHandle, filename.c_str(), &zi, nullptr, 0, nullptr, 0, nullptr, Z_DEFLATED, Z_DEFAULT_COMPRESSION);
        code != ZIP_OK) {
        throw AZLibException("zipOpenNewFileInZip failed: {}"_format(code));
    }
    AUI_DEFER {
        zipCloseFileInZip(mHandle);
    };
    struct ZipFileEntry: IOutputStream {
        zipFile handle;

        ZipFileEntry() = default;
        ~ZipFileEntry() override = default;

        void write(const char* src, size_t size) override {
            if (auto code = zipWriteInFileInZip(handle, src, size); code != ZIP_OK) {
                throw AZLibException("zipWriteInFileInZip failed: {}"_format(code));
            }
        }
    } os;
    os.handle = mHandle;
    visitor(os);
}

} // namespace aui::archive::zip
