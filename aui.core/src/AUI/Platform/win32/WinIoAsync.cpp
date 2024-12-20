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
// Created by Alex2772 on 4/19/2022.
//

#include "WinIoAsync.h"
#include "WinIoThread.h"
#include <AUI/Thread/AFuture.h>

class WinIoAsync::Impl: public std::enable_shared_from_this<WinIoAsync::Impl> {
public:
    void init(HANDLE fileHandle, std::function<void(const AByteBuffer&)> callback) {
        mCallback = std::move(callback);
        auto self = shared_from_this();
        AFuture<> cs;
        WinIoThread::enqueue([&, self = std::move(self), fileHandle] {
            self->mBuffer.resize(0x1000);
            self->mOverlapped.Pointer = self.get();
            self->mFileHandle = fileHandle;
            nextRead();
            cs.supplyValue();
        });
        cs.wait();
    }


    void handleCallback(DWORD dwErrorCode, DWORD bytesTransferred) noexcept {
        mBuffer.setSize(bytesTransferred);
        switch (dwErrorCode) {
            case 0:
                mCallback(mBuffer);
                nextRead();
                break;

            case ERROR_OPERATION_ABORTED:
            case ERROR_BROKEN_PIPE:
                mCallback(mBuffer);
                break;

            default:
                AUI_ASSERT(!("WinIoAsync failed"));
        }
    }

    ~Impl() {
        if (mFileHandle) {
            auto r = CancelIoEx(mFileHandle, &mOverlapped);
            auto err = GetLastError();
            if (err != ERROR_NOT_FOUND) {
                AUI_ASSERTX(r, "CancelIo failed");
            }

            r = CloseHandle(mFileHandle);
            AUI_ASSERTX(r, "CloseHandle failed");
        }
    }

private:
    HANDLE mFileHandle = nullptr;
    AByteBuffer mBuffer;
    OVERLAPPED mOverlapped;
    std::function<void(const AByteBuffer&)> mCallback;

    void nextRead() {
        if (!ReadFileEx(mFileHandle,
                        mBuffer.data(),
                        mBuffer.capacity(),
                        &mOverlapped,
                        [](DWORD dwErrorCode, DWORD bytesTransferred, LPOVERLAPPED overlapped) {
                            reinterpret_cast<WinIoAsync::Impl*>(overlapped->Pointer)->handleCallback(dwErrorCode, bytesTransferred);
                        })) {
            throw AException("ReadFileEx failed");
        }
        if (GetLastError() != ERROR_SUCCESS) {
            throw AException("ReadFileEx failed");
        }
    }
};

void WinIoAsync::init(HANDLE fileHandle, std::function<void(const AByteBuffer&)> callback) {
    AUI_ASSERTX(mImpl == nullptr, "already initialized");
    mImpl = _new<Impl>();
    mImpl->init(fileHandle, std::move(callback));
}

