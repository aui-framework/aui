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

class API_AUI_CORE AObjectBase {
    /* ASignal <-> AObject implementation stuff */
    friend class AAbstractSignal;

    /* tests */
    friend class SignalSlotTest;
    friend class PropertyTest;

public:
    AObjectBase() = default;

    static ASpinlockMutex SIGNAL_SLOT_GLOBAL_SYNC;

    AObjectBase(AObjectBase&& rhs) noexcept {
        operator=(std::move(rhs));
    }

    AObjectBase(const AObjectBase& rhs) noexcept {
        // mIngoingConnections are not borrowed on copy operation.
    }

    AObjectBase& operator=(const AObjectBase& rhs) noexcept {
        // mIngoingConnections are not borrowed on copy operation.
        return *this;
    }

    AObjectBase& operator=(AObjectBase&& rhs) noexcept {
        if (this == &rhs) {
            return *this;
        }
        AUI_ASSERTX(rhs.mIngoingConnections.empty(), "AObjectBase move is valid only if no signals connected to it");
        return *this;
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
        ReceiverConnectionOwner& operator=(const ReceiverConnectionOwner& rhs) {
            if (this == &rhs) {
                return *this;
            }
            release();
            value = rhs.value;
            return *this;
        }

        ReceiverConnectionOwner& operator=(ReceiverConnectionOwner&& rhs) noexcept {
            if (this == &rhs) {
                return *this;
            }
            release();
            value = std::move(rhs.value);
            return *this;
        }

        ~ReceiverConnectionOwner() {
            release();
        }

    private:
        void release() noexcept {
            if (!value) {
                return;
            }
            value->onBeforeReceiverSideDestroyed();
        }
    };

    AVector<ReceiverConnectionOwner> mIngoingConnections;
};
