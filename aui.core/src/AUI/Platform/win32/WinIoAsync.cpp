// AUI Framework - Declarative UI toolkit for modern C++20
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
// Created by Alex2772 on 4/19/2022.
//

#include "WinIoAsync.h"
#include "WinIoThread.h"
#include "AUI/Thread/ACutoffSignal.h"
#include <AUI/Thread/AFuture.h>

class WinIoAsync::Impl: public std::enable_shared_from_this<WinIoAsync::Impl> {
public:
    void init(HANDLE fileHandle, std::function<void(const AByteBuffer&)> callback) {
        mCallback = std::move(callback);
        auto self = shared_from_this();
        ACutoffSignal cutoff;
        WinIoThread::enqueue([&, self = std::move(self), fileHandle] {
            self->mBuffer.resize(0x1000);
            self->mOverlapped.Pointer = self.get();
            self->mFileHandle = fileHandle;
            nextRead();
            cutoff.makeSignal();
        });
        cutoff.waitForSignal();
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
                assert(!("WinIoAsync failed"));
        }
    }

    ~Impl() {
        if (mFileHandle) {
            auto r = CancelIoEx(mFileHandle, &mOverlapped);
            auto err = GetLastError();
            if (err != ERROR_NOT_FOUND) {
                assert(("CancelIo failed", r));
            }

            r = CloseHandle(mFileHandle);
            assert(("CloseHandle failed", r));
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
    assert(("already initialized", mImpl == nullptr));
    mImpl = _new<Impl>();
    mImpl->init(fileHandle, std::move(callback));
}

