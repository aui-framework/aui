// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2024 Alex2772 and Contributors
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
#include "AUI/Util/Assert.h"
#include "UnixIoThread.h"
#include <AUI/Thread/AFuture.h>
#include <unistd.h>

class UnixIoAsync::Impl: public std::enable_shared_from_this<UnixIoAsync::Impl> {
public:
    void init(int fileHandle, std::function<void(const AByteBuffer&)> callback) {
        AUI_ASSERT(fileHandle != 0);

        mCallback = std::move(callback);
        mFileHandle = fileHandle;
        mBuffer.reserve(0x1000);
        UnixIoThread::inst().registerCallback(fileHandle, UnixPollEvent::IN, [self = weak_from_this()](ABitField<UnixPollEvent>) {
            auto s = self.lock();
            AUI_ASSERTX(s != nullptr, "UnixIoAsync::Impl is dead, but callback is called");
            AUI_ASSERT(s->mBuffer.data() != nullptr);
            auto r = read(s->mFileHandle, s->mBuffer.data(), s->mBuffer.capacity());
            AUI_ASSERT(r >= 0);
            s->mBuffer.setSize(r);
            s->mCallback(s->mBuffer);
        });
    }


    ~Impl() {
        AUI_ASSERT(mFileHandle != 0);
        UnixIoThread::inst().unregisterCallback(mFileHandle);
    }

private:
    int mFileHandle;
    AByteBuffer mBuffer;
    std::function<void(const AByteBuffer&)> mCallback;

};

void UnixIoAsync::init(int fileHandle, std::function<void(const AByteBuffer&)> callback) {
    AUI_ASSERTX(mImpl == nullptr, "already initialized");
    mImpl = _new<Impl>();
    mImpl->init(fileHandle, std::move(callback));
}

