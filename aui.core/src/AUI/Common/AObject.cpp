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
