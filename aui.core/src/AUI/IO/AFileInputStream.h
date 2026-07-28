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
#include "AUI/Core.h"
#include "IInputStream.h"
#include "ISeekableInputStream.h"

class AString;

/**
 * @brief Opens a file for a binary read.
 * @ingroup io
 */
class API_AUI_CORE AFileInputStream: public ISeekableInputStream
{
private:
    FILE* mFile = nullptr;

public:
    AFileInputStream(const AString& path);
    virtual ~AFileInputStream();

    AFileInputStream(AFileInputStream&& rhs) noexcept {
        operator=(std::move(rhs));
    }
    AFileInputStream& operator=(AFileInputStream&& rhs) noexcept {
        mFile = rhs.mFile;
        rhs.mFile = nullptr;
        return *this;
    }

    FILE* nativeHandle() const { return mFile; }

    void seek(std::streamoff offset, ASeekDir seekDir) override;
    [[nodiscard]] std::streampos tell() noexcept override;
    bool isEof() override;
    std::size_t read(char* dst, size_t size) override;
};
