//  AUI Framework - Declarative UI toolkit for modern C++20
//  Copyright (C) 2020-2023 Alex2772
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library. If not, see <http://www.gnu.org/licenses/>.

#pragma once

#include "AUI/Audio/ISoundInputStream.h"
#include "AUI/Common/AByteBuffer.h"
#include "AUI/IO/AStrongByteBufferInputStream.h"

/**
 * @brief Sound stream for storing raw sound data, useful for storing recorded sound data
 * @ingroup audio
 */
class ARawSoundStream : public ISoundInputStream {
public:
    ARawSoundStream(AAudioFormat format, AByteBuffer data) : mFormat(std::move(format)),
                                                             mStream(_new<AStrongByteBufferInputStream>(std::move(data))) { }

    size_t read(char* dst, size_t size) override {
        return mStream->read(dst, size);
    }

    AAudioFormat info() override {
        return mFormat;
    }

    void rewind() override {
        mStream->seek(0, std::ios::beg);
    }

private:
    AAudioFormat mFormat;
    _<AStrongByteBufferInputStream> mStream;
};