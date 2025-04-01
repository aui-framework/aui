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

#pragma once

#include <istream>
#include "IInputStream.h"


/**
 * @brief std::istream implementation wrapping AUI's IInputStream.
 * @ingroup io
 */
class API_AUI_CORE AStdIStream: public std::istream {
public:
    class StreamBuf: public std::streambuf  {
    public:
        StreamBuf(_<IInputStream> is);

        virtual ~StreamBuf();

    protected:
        int_type underflow() override;

    private:
        _<IInputStream> mIs;

        char mBuffer[0x1000]{};
    };

    explicit AStdIStream(_<IInputStream> is);
    ~AStdIStream() {
        delete rdbuf();
    }
};