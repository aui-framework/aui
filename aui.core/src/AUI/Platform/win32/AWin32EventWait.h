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

#pragma once

#include <Windows.h>
#include <functional>
#include <cassert>

/**
 * @brief Encapsulates calls to RegisterWaitForSingleObject/UnregisterWait
 */
class API_AUI_CORE AWin32EventWait: public aui::noncopyable {
private:
    HANDLE mNewWaitObject = nullptr;
    std::function<void()> mCallback;

public:
    AWin32EventWait() noexcept = default;
    ~AWin32EventWait() {
        if (mNewWaitObject != nullptr) {
            auto r = UnregisterWaitEx(mNewWaitObject, INVALID_HANDLE_VALUE);
            AUI_ASSERT(r != 0);
        }
    }

    void registerWaitForSingleObject(HANDLE baseHandle, std::function<void()> callback, DWORD timeout = INFINITE, DWORD flags = WT_EXECUTEDEFAULT) noexcept {
        AUI_ASSERTX(mNewWaitObject == nullptr, "waitForExitCode object already registered");

        mCallback = std::move(callback);

        auto r = RegisterWaitForSingleObject(&mNewWaitObject, baseHandle, [](PVOID context, BOOLEAN){
            auto self = reinterpret_cast<AWin32EventWait*>(context);
            self->mCallback();
        }, this, timeout, flags);
        AUI_ASSERT(r != 0);
    }
};


