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

#include <range/v3/algorithm/find.hpp>

#include "AAbstractSignal.h"
#include "AObject.h"

void AAbstractSignal::addIngoingConnectionIn(aui::no_escape<AObjectBase> object, _<Connection> connection) {
    std::unique_lock lock(AObjectBase::SIGNAL_SLOT_GLOBAL_SYNC);
    object->mIngoingConnections.emplace_back(std::move(connection));
}

void AAbstractSignal::removeIngoingConnectionIn(aui::no_escape<AObjectBase> object, Connection& connection, std::unique_lock<ASpinlockMutex>& lock) {
    auto c = [&]() -> _<Connection> {
        auto it = ranges::find(object->mIngoingConnections, &connection, [](const auto& v) { return v.value.get(); });
        if (it == object->mIngoingConnections.end()) {
            return nullptr;
        }
        auto value = std::exchange(it->value, nullptr);
        object->mIngoingConnections.erase(it);
        lock.unlock();
        return value;
    }();
    c.reset();
}

_weak<AObject> AAbstractSignal::weakPtrFromObject(AObject* object) {
    try {
        return aui::ptr::weak_from_this(object);
    } catch (const std::bad_weak_ptr&) {
        return {};
    }
}
