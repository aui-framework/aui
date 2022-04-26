//
// Created by Alex2772 on 4/19/2022.
//

#include "UnixIoAsync.h"
#include "UnixIoThread.h"
#include <AUI/Thread/AFuture.h>
#include <unistd.h>

class UnixIoAsync::Impl: public std::enable_shared_from_this<UnixIoAsync::Impl> {
public:
    void init(int fileHandle, std::function<void(const AByteBuffer&)> callback) {
        mCallback = std::move(callback);
        mFileHandle = fileHandle;
        auto self = shared_from_this();
        mBuffer.reserve(0x1000);
        UnixIoThread::inst().registerCallback(fileHandle, POLLIN, [&](int) {
            auto r = read(mFileHandle, mBuffer.data(), mBuffer.capacity());
            assert(r >= 0);
            mBuffer.setSize(r);
            mCallback(mBuffer);
        });
    }


    ~Impl() {
        if (mFileHandle) {
            UnixIoThread::inst().unregisterCallback(mFileHandle);
        }
    }

private:
    int mFileHandle;
    AByteBuffer mBuffer;
    std::function<void(const AByteBuffer&)> mCallback;

};

void UnixIoAsync::init(int fileHandle, std::function<void(const AByteBuffer&)> callback) {
    assert(("already initialized", mImpl == nullptr));
    mImpl = _new<Impl>();
    mImpl->init(fileHandle, std::move(callback));
}

