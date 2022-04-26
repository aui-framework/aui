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
        AFuture<> future;
        WinIoThread::enqueue([&, self = std::move(self), fileHandle] {
            self->mBuffer.resize(0x1000);
            self->mOverlapped.Pointer = self.get();
            self->mFileHandle = fileHandle;
            nextRead();
            future.supplyResult();
        });
        future.wait();
    }


    void handleCallback(DWORD dwErrorCode, DWORD bytesTransferred) noexcept {
        mBuffer.setSize(bytesTransferred);
        switch (dwErrorCode) {
            case 0:
                mCallback(mBuffer);
                nextRead();
                break;
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

