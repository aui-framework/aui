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

#include "React.h"
#include <AUI/Common/AObject.h>
#include <AUI/Common/AAbstractSignal.h>

using namespace aui::react;

namespace {
thread_local DependencyObserver* gCurrentDependencyObserver = nullptr;
}

DependencyObserverScope::DependencyObserverScope(DependencyObserver* observer)
  : mPrevObserver(std::exchange(gCurrentDependencyObserver, observer)) {
    AUI_NULLSAFE(observer)->mObserverConnections.clear();
}

DependencyObserverScope::~DependencyObserverScope() { gCurrentDependencyObserver = mPrevObserver; }

void DependencyObserverScope::addDependency(const AAbstractSignal& signal) {
    if (!gCurrentDependencyObserver) {
        return;
    }
    if (signal.hasOutgoingConnectionsWith(gCurrentDependencyObserver)) {
        return;
    }
    auto connection = const_cast<AAbstractSignal&>(signal).addGenericObserver(
        gCurrentDependencyObserver, [observer = gCurrentDependencyObserver] { observer->invalidate(); });
    gCurrentDependencyObserver->mObserverConnections << std::move(connection);
}
