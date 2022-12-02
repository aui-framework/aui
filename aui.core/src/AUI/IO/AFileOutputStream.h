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
#include "IOutputStream.h"
#include "AUI/Core.h"

class AString;

/**
 * @brief Opens a file for a binary write.
 * @ingroup io
 */
class API_AUI_CORE AFileOutputStream : public IOutputStream
{
private:
    FILE* mFile;
    AString mPath;

public:
    AFileOutputStream(): mFile(nullptr) {}
    AFileOutputStream(AString path, bool append = false);

    virtual ~AFileOutputStream();

    void write(const char* src, size_t size) override;
    void close();


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
