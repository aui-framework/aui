/*
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
 */

#pragma once
#include <cstdio>
#include "IOutputStream.h"
#include "AUI/Core.h"

class AString;

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
