// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2025 Alex2772 and Contributors
//
// SPDX-License-Identifier: MPL-2.0
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <AUI/IO/APath.h>
#include <AUI/IO/ISeekableInputStream.h>
#include <AUI/IO/ISeekableOutputStream.h>

namespace aui::archive {

/**
 * @brief Zip file entry.
 */
struct API_AUI_CORE FileEntry : aui::noncopyable {
    /**
     * @brief file path inside ZIP.
     */
    AStringView name;

    /**
     * @brief Opens the zip entry for read.
     * @param password
     * @return
     */
    virtual aui::no_escape<IInputStream> open(const std::string& password = {}) const = 0;

};

/**
 * @brief Default extracter for aui::zlib::readZip.
 * @ingroup io
 */
struct API_AUI_CORE ExtractTo {
    /**
     * @brief Destination dir to unpack.
     */
    APath prefix;

    /**
     * @brief Function to preprocess the destination path (excluding prefix).
     */
    std::function<APath(APath)> pathProjection = aui::identity{};

    void operator()(const FileEntry& zipEntry) const;
};

namespace zip {
/**
 * @brief ZIP reader.
 * @ingroup io
 * @param stream ZIP file stream.
 * @param visitor zip contents visitor.
 * @details
 * ZIP file names are passed to callback. Optionally, the callback might request file contents by FileEntry::open().
 * @sa aui::zlib::ExtractTo
 */
void API_AUI_CORE read(aui::no_escape<ISeekableInputStream> stream, const std::function<void(const FileEntry&)>& visitor);

/**
 * @brief Writer for ZIP archives.
 * @ingroup io
 * @details
 * The writer is stateful and not thread‑safe. It throws on the first failure.
 */
class API_AUI_CORE Writer: public aui::noncopyable {
public:
    explicit Writer(_unique<ISeekableOutputStream> to);
    ~Writer();

    /**
     * @brief Creates a new file within a zip.
     * @param path path of file within archive.
     * @param visitor Consumer lambda of output stream. The lambda writes data to the provided output stream.
     */
    void openFileInZip(const APath& path, const std::function<void(IOutputStream&)>& visitor) const;

private:
    _unique<ISeekableOutputStream> mZipFile;
    void* mHandle;
};
}
}
