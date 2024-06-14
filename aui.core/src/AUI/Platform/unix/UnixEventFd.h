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
// Created by alex2772 on 4/19/22.
//

#pragma once


#include "AUI/Traits/values.h"

class UnixEventFd: public aui::noncopyable {
public:
    UnixEventFd() noexcept;
    ~UnixEventFd();

    void set() noexcept;
    void reset() noexcept;

    [[nodiscard]]
    int handle() noexcept {
#if AUI_PLATFORM_APPLE
        return mIn;
#else
        return mHandle;
#endif
    }

private:
#if AUI_PLATFORM_APPLE
    int mOut, mIn;
#else
    int mHandle;
#endif
};



