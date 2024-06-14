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

#pragma once
#include <cstdio>
#include "AUI/Core.h"
#include "IInputStream.h"

class AString;

/**
 * @brief Opens a file for a binary read.
 * @ingroup io
 */
class API_AUI_CORE AFileInputStream: public IInputStream
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

    FILE* nativeHandle() const {
        return mFile;
    }

    enum class Seek {
        /**
         * Seek relatively to the begin of file
         */
        BEGIN,

        /**
         * Seek relatively to the current position
         */
        CURRENT,

        /**
         * Seek relative to the end of file
         */
        END
    };

    void seek(std::streamoff offset, Seek dir) noexcept;
    void seek(std::streampos pos) noexcept;
    std::streampos tell() noexcept;

    std::size_t size() noexcept {
        auto current = tell();
        seek(0, Seek::END);
        auto size = tell();
        seek(current, Seek::BEGIN);
        return size;
    }

    std::size_t read(char* dst, size_t size) override;
};
