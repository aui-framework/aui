// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2024 Alex2772 and Contributors
//
// SPDX-License-Identifier: MPL-2.0
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <AUI/IO/APath.h>
#include <AUI/IO/ISeekableInputStream.h>

namespace aui::archive {

/**
 * @brief Zip file entry.
 */
struct API_AUI_CORE FileEntry : aui::noncopyable {
    friend void API_AUI_CORE
    readZip(aui::no_escape<ISeekableInputStream> stream, const std::function<void(const FileEntry&)>& visitor);
    /**
     * @brief file path inside ZIP.
     */
    std::string_view name;

    struct API_AUI_CORE Stream : IInputStream, aui::noncopyable {
        friend struct API_AUI_CORE FileEntry;
        size_t read(char* dst, size_t size) override;
        ~Stream() override;

    private:
        explicit Stream(void* handle) : mHandle(handle) {}
        void* mHandle = nullptr;
    };

    /**
     * @brief Opens the zip entry for read.
     * @param password
     * @return
     */
    Stream open(const std::string& password = {}) const;

private:
    void* mHandle;
};

/**
 * @brief Default extracter for aui::zlib::readZip.
 * @ingroup io
 */
struct API_AUI_CORE ExtractTo {
    APath prefix;

    void operator()(const FileEntry& zipEntry) const;
};

namespace zip {
/**
 * @brief ZIP reader.
 * @ingroup io
 * @param stream ZIP file stream.
 * @param visitor zip contents visitor.
 * @details
 * ZIP file names are passed to callback. Optionally, the callback might request file contents by ZipEntry::open().
 * @sa aui::zlib::ExtractTo
 */
void API_AUI_CORE read(aui::no_escape<ISeekableInputStream> stream, const std::function<void(const FileEntry&)>& visitor);
}
}
