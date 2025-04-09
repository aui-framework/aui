/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2025 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "IEventLoop.h"
#include "AThread.h"

IEventLoop::Handle::Handle(IEventLoop* loop) : mCurrentEventLoop(loop) {
	std::unique_lock lock(AThread::current()->mEventLoopLock);
	mPrevEventLoop = AThread::current()->mCurrentEventLoop;
	AThread::current()->mCurrentEventLoop = loop;
}

IEventLoop::Handle::~Handle() {
	std::unique_lock lock(AThread::current()->mEventLoopLock);
	AThread::current()->mCurrentEventLoop = mPrevEventLoop;
}
