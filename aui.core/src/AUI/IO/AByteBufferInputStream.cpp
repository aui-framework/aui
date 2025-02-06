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

#include "AByteBufferInputStream.h"
#include <glm/glm.hpp>

size_t AByteBufferInputStream::read(char* dst, size_t size) {
    auto last = mCurrent + size;
    if (last > mEnd) {
        last = mEnd;
    }
    auto sizeToCopy = last - mCurrent;
    std::memcpy(dst, mCurrent, sizeToCopy);
    mCurrent = last;
    return sizeToCopy;
}

void AByteBufferInputStream::seek(std::streamoff offset, ASeekDir seekDir) {
    switch (seekDir) {
        case ASeekDir::BEGIN:
            mCurrent = mBegin + offset;
            break;
        case ASeekDir::CURRENT:
            mCurrent += offset;
            break;
        case ASeekDir::END:
            mCurrent = mEnd + offset;
            break;
        default:
            break;
    }
    mCurrent = std::clamp(mCurrent, mBegin, mEnd);
}

std::streampos AByteBufferInputStream::tell() noexcept { return std::distance(mBegin, mCurrent); }

bool AByteBufferInputStream::isEof() {
    return mCurrent == mEnd;
}
