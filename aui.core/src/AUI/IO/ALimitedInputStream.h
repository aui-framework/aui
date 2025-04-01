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
// Created by alex2 on 16.11.2020.
//

#pragma once


#include "IInputStream.h"

/**
 * @brief Input stream that will pass through a maximum of a certain number of bytes - then it will return eof
 *        (end of stream)
 * @ingroup io
 */
class API_AUI_CORE ALimitedInputStream: public IInputStream {
private:
    _<IInputStream> mInputStream;
    size_t mLimit;

public:
    ALimitedInputStream(const _<IInputStream>& inputStream, size_t limit) : mInputStream(inputStream), mLimit(limit) {}
    virtual ~ALimitedInputStream() = default;

    size_t read(char* dst, size_t size) override;
};


