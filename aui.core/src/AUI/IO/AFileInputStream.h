// AUI Framework - Declarative UI toolkit for modern C++17
// Copyright (C) 2020-2022 Alex2772
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
    FILE* mFile;

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
