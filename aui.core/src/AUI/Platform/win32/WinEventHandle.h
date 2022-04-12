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
            UnregisterWait(mNewWaitObject);
        }
    }

    void registerWaitForSingleObject(HANDLE baseHandle, std::function<void()> callback, DWORD timeout = INFINITE, DWORD flags = WT_EXECUTEDEFAULT) noexcept {
        assert(("waitForExitCode object already registered", mNewWaitObject == nullptr));

        mCallback = std::move(callback);

        auto r = RegisterWaitForSingleObject(&mNewWaitObject, baseHandle, [](PVOID context, BOOLEAN){
            reinterpret_cast<WinEventHandle*>(context)->mCallback();
        }, this, timeout, flags);
        assert(r != 0);
    }
};


