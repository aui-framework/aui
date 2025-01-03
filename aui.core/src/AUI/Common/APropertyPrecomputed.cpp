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

#include "APropertyPrecomputed.h"
#include <AUI/Common/AObject.h>
#include <AUI/Common/AAbstractSignal.h>

using namespace aui::property_precomputed;

namespace {
thread_local detail::DependencyObserver* gCurrentDependencyObserver = nullptr;
}

detail::DependencyObserverRegistrar::DependencyObserverRegistrar(detail::DependencyObserver& observer)
  : mPrevObserver(std::exchange(gCurrentDependencyObserver, &observer)) {}

detail::DependencyObserverRegistrar::~DependencyObserverRegistrar() { gCurrentDependencyObserver = mPrevObserver; }

void ::aui::property_precomputed::addDependency(const AAbstractSignal& signal) {
    if (!gCurrentDependencyObserver) {
        return;
    }
    if (signal.hasConnectionsWith(gCurrentDependencyObserver)) {
        return;
    }
    const_cast<AAbstractSignal&>(signal).addGenericObserver(
        gCurrentDependencyObserver, [observer = gCurrentDependencyObserver] { observer->invalidate(); });
}
