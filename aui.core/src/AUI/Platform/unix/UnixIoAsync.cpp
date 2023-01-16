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

