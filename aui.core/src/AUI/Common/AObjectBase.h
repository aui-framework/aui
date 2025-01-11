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

#pragma once

#include <AUI/Thread/AMutex.h>
#include <AUI/Traits/concepts.h>
#include <AUI/Traits/members.h>

#include "AUI/Common/AVector.h"
#include "AUI/Core.h"
#include "AUI/Traits/values.h"
#include "SharedPtrTypes.h"
#include <AUI/Common/AAbstractSignal.h>

class API_AUI_CORE AObjectBase : public aui::noncopyable {
    /* ASignal <-> AObject implementation stuff */
    friend class AAbstractSignal;

    /* tests */
    friend class SignalSlotTest;

public:
    AObjectBase() = default;

    static ASpinlockMutex SIGNAL_SLOT_GLOBAL_SYNC;

    AObjectBase(AObjectBase&& rhs) noexcept {
        AUI_ASSERTX(rhs.mIngoingConnections.empty(), "AObjectBase move is valid only if no signals connected to it");
    }

protected:
    void clearAllIngoingConnections() noexcept;

private:
    /**
     * @brief Connection owner which destroys the connection in destructor.
     */
    struct ReceiverConnectionOwner {
        _<AAbstractSignal::Connection> value = nullptr;

        ReceiverConnectionOwner() = default;
        explicit ReceiverConnectionOwner(_<AAbstractSignal::Connection> connection) noexcept
          : value(std::move(connection)) {}
        ReceiverConnectionOwner(const ReceiverConnectionOwner&) = default;
        ReceiverConnectionOwner(ReceiverConnectionOwner&&) noexcept = default;
        ReceiverConnectionOwner& operator=(const ReceiverConnectionOwner&) = default;
        ReceiverConnectionOwner& operator=(ReceiverConnectionOwner&&) noexcept = default;

        ~ReceiverConnectionOwner() {
            if (value) {
                value->onBeforeReceiverSideDestroyed();
            }
        }
    };

    AVector<ReceiverConnectionOwner> mIngoingConnections;
};
