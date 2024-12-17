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

