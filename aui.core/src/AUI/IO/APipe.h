/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2025 Alex2772 and Contributors
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

#pragma once


#include "IInputStream.h"
#include "IOutputStream.h"
#include <AUI/Thread/AConditionVariable.h>

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


