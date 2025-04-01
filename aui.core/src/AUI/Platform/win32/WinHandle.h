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

#include <Windows.h>
#include "AUI/Thread/AThread.h"

namespace aui::win32 {
class Handle: public aui::noncopyable {
public:
    Handle(HANDLE handle): mHandle(handle) {}
    ~Handle() {
       CloseHandle(mHandle);
    }
 
    [[nodiscard]] 
    operator HANDLE() const noexcept {
        return mHandle; 
    }
    

private:
    HANDLE mHandle;

};
}
