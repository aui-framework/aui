// AUI Framework - Declarative UI toolkit for modern C++17
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

#pragma once


#include "IInputStream.h"
#include "IOutputStream.h"

/**
 * @brief A thread safe buffer that converts an IInputStream to IOutputStream (and otherwise).
 * @ingroup core
 * @ingroup io
 * @details
 * APipe is similar to the unix pipes. It's capable to store up to 65,536 bytes. It's implemented like a ring buffer.
 * If this limit is reached, a write() method blocks thread until buffer is read from.
 *
 * If capacity may be not enough, consider to use ADynamicPipe instead.
 */
class API_AUI_CORE APipe: public IInputStream, public IOutputStream {
private:
    char mCircularBuffer[0x10000];
    uint16_t mReaderPos = 0;
    uint16_t mWriterPos = 0;

    AMutex mMutex;
    AConditionVariable mConditionVariable;

    bool mClosed = false;

public:
    APipe();
    virtual ~APipe();

    size_t read(char* dst, size_t size) override;
    void write(const char* src, size_t size) override;

    size_t available();

    void close();

};


