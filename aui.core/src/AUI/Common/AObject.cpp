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

#include "AObject.h"
#include "AAbstractSignal.h"
#include "AUI/Thread/AThread.h"


thread_local bool gIsDisconnected;

void AObject::disconnect()
{
    gIsDisconnected = true;
}

AObject::AObject()
{
	mAttachedThread = AThread::current();
}

AObject::~AObject()
{
	clearSignals();
}

void AObject::clearSignals() noexcept
{
    std::unique_lock lock(mSignalsLock);
	for (auto it = mSignals.begin(); !mSignals.empty();  it = mSignals.begin())
	{
        auto signal = *it;
        mSignals.erase(it);
	    if (!signal->isDestroyed()) {
            lock.unlock();
            signal->clearAllConnectionsWith(this);
            lock.lock();
        }
	}
}

bool& AObject::isDisconnected() {
    return gIsDisconnected;
}

void AObject::moveToThread(aui::no_escape<AObject> object, _<AAbstractThread> thread) {
    object->setThread(std::move(thread));
}
