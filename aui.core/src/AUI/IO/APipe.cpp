// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2023 Alex2772
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
