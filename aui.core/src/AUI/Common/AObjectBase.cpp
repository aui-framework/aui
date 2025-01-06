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

static ASpinlockMutex gSync;

void AObjectBase::clearSignals() noexcept {
    auto signals = [&] {
      std::unique_lock lock(gSync);
      return std::exchange(mSignals, {});
    }();
    for (const auto& signal : signals) {
        if (!signal->isDestroyed()) {
            signal->clearAllConnectionsWith(this);
        }
    }
}

void AAbstractSignal::linkSlot(AObjectBase* object) noexcept {
    std::unique_lock lock(gSync);
    object->mSignals.insert(this);
}

void AAbstractSignal::unlinkSlot(AObjectBase* object) noexcept {
    std::unique_lock lock(gSync);
    object->mSignals.erase(this);
}
