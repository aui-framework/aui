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

#include "AObject.h"
#include "AAbstractSignal.h"
#include "AObjectBase.h"
#include "AUI/Logging/ALogger.h"

ASpinlockMutex AObjectBase::SIGNAL_SLOT_GLOBAL_SYNC;

void AObjectBase::clearAllIngoingConnections() noexcept {
    auto incomingConnections = [&] {
      std::unique_lock lock(SIGNAL_SLOT_GLOBAL_SYNC);
      return std::exchange(mIngoingConnections, {});
    }();
    incomingConnections.clear();
}

void AObjectBase::handleSlotException(std::exception_ptr exception) {
    auto l = ALogger::info("AObject");
    l << "An exception has occurred during signal processing emitted by this object: ";
    try {
        std::rethrow_exception(exception);
    } catch (const AException& e) {
        l << e;
    } catch (const std::exception& e) {
        l << e.what();
    }
}
