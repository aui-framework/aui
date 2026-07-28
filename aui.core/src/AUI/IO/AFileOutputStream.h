/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2025 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once
#include <cstdio>
#include "ISeekableOutputStream.h"
#include "AUI/Core.h"

class AString;

/**
 * @brief Opens a file for a binary write.
 * @ingroup io
 */
class API_AUI_CORE AFileOutputStream : public aui::ISeekableOutputStream
{
private:
    FILE* mFile;
    AString mPath;

public:
    AFileOutputStream(): mFile(nullptr) {}
    AFileOutputStream(AString path, bool append = false);

    virtual ~AFileOutputStream();

    void seek(std::streamoff offset, ASeekDir seekDir) override;
    std::streampos tell() noexcept override;
    bool isEof() override;

    void write(const char* src, size_t size) override;
    void close();
    void open(bool append = false);


    AFileOutputStream(AFileOutputStream&& rhs) noexcept {
        operator=(std::move(rhs));
    }
    AFileOutputStream& operator=(AFileOutputStream&& rhs) noexcept {
        mFile = rhs.mFile;
        mPath = std::move(rhs.mPath);
        rhs.mFile = nullptr;
        return *this;
    }

    FILE* nativeHandle() const {
        return mFile;
    }
    const AString& path() const {
        return mPath;
    }

    /**
     * Probably thrown when target storage went out of space
     */
    class WriteException: public AIOException {
    private:
        AString mPath;

    public:
        WriteException(const AString &mPath) : mPath(mPath) {}

        const AString& getPath() const {
            return mPath;
        }

        AString getMessage() const noexcept override {
            return "failed to write to file: " + mPath;
        }
    };
};
