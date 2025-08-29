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

#include "AJson.h"
#include <range/v3/algorithm/find.hpp>
#include "AUI/IO/AStringStream.h"
#include "AUI/Common/AByteBuffer.h"
#include "AUI/IO/AByteBufferInputStream.h"

AString AJson::toString(const AJson& json) {
    AByteBuffer buffer;
    aui::serialize(buffer, json);
    return AString(buffer.data(), buffer.size());
}

AJson AJson::fromString(const AString& json) {
    AStringStream sis(json);
    return aui::deserialize<AJson>(sis);
}

AJson AJson::fromBuffer(AByteBufferView buffer) {
    try {
        return aui::deserialize<AJson>(AByteBufferInputStream(buffer));
    } catch (...) {
        throw AJsonException("While parsing:\n" + AString(buffer.data(), buffer.size()), std::current_exception());
    }
}

AJson AJson::mergedWith(const AJson &other) {
    auto& thisObject = as<Object>();
    auto& otherObject = other.as<Object>();
    Object thisCopy = thisObject;
    for (const auto& [key, value] : otherObject) {
        thisCopy[key] = value;
    }

    return thisCopy;
}

std::pair<AString, AJson>* aui::impl::JsonObject::contains(const AString& key) noexcept {
    if (auto it = ranges::find(*this, key, &std::pair<AString, AJson>::first); it != end()) {
        return &(*it);
    }
    return nullptr;
}

AJson& aui::impl::JsonObject::operator[](const AString& key) {
    if (auto v = contains(key)) {
        return v->second;
    }
    emplace_back(key, AJson{});
    return back().second;
}

AJson& aui::impl::JsonObject::at(const AString& key) {
    if (auto v = contains(key)) {
        return v->second;
    }
    throw AException("no such key: {}"_format(key));
}
