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

#pragma once

#include <Windows.h>
#include <functional>
#include <cassert>

/**
 * Encapsulates calls to RegisterWaitForSingleObject/UnregisterWait
 */
class WinEventHandle {
private:
    HANDLE mNewWaitObject = nullptr;
    std::function<void()> mCallback;

public:
    WinEventHandle() noexcept = default;
    ~WinEventHandle() {
        if (mNewWaitObject != nullptr) {
            auto r = UnregisterWaitEx(mNewWaitObject, INVALID_HANDLE_VALUE);
            assert(r != 0);
        }
    }

    void registerWaitForSingleObject(HANDLE baseHandle, std::function<void()> callback, DWORD timeout = INFINITE, DWORD flags = WT_EXECUTEDEFAULT) noexcept {
        assert(("waitForExitCode object already registered", mNewWaitObject == nullptr));

        mCallback = std::move(callback);

        auto r = RegisterWaitForSingleObject(&mNewWaitObject, baseHandle, [](PVOID context, BOOLEAN){
            auto self = reinterpret_cast<WinEventHandle*>(context);
            self->mCallback();
        }, this, timeout, flags);
        assert(r != 0);
    }
};


