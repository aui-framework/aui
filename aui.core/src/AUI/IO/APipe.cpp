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

//
// Created by alex2 on 15.11.2020.
//

#include "APipe.h"

using namespace std::chrono_literals;

size_t APipe::read(char* dst, size_t size) {
    std::unique_lock lock(mMutex);
    int read = 0;
    for (uint16_t i = 0; i < size; ++i, ++mReaderPos, ++read) {
        while (mWriterPos == mReaderPos) {
            if (read)
                return read;
            mConditionVariable.wait_for(lock, 100ms);
            if (mClosed) {
                return 0;
            }
        }
        dst[i] = mCircularBuffer[mReaderPos];
    }
    return read;
}

void APipe::write(const char* src, size_t size) {
    std::unique_lock lock(mMutex);
    if (mClosed) {
        throw AException("pipe is closed");
    }
    for (uint16_t i = 0; i < size; ++i, ++mWriterPos) {
        while (mWriterPos + 1 == mReaderPos) {
            mConditionVariable.notify_one();
            mConditionVariable.wait_for(lock, 100ms);
            if (mClosed) {
                throw AException("pipe is closed");
            }
        }
        mCircularBuffer[mWriterPos] = src[i];
    }
    mConditionVariable.notify_one();
}

void APipe::close() {
    mClosed = true;
    std::unique_lock lock(mMutex);
    mConditionVariable.notify_all();
}

APipe::~APipe() {
    close();
}

APipe::APipe() {

}

size_t APipe::available() {
    return mWriterPos - mReaderPos;
}
