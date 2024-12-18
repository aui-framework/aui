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
