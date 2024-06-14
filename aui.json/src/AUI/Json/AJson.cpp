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

#include "AJson.h"
#include "AUI/IO/AStringStream.h"
#include "AUI/Common/AByteBuffer.h"
#include "AUI/IO/AByteBufferInputStream.h"

AString AJson::toString(const AJson& json) {
    AByteBuffer buffer;
    aui::serialize(buffer, json);
    return AString::fromUtf8(buffer);
}

AJson AJson::fromString(const AString& json) {
    AStringStream sis(json);
    return aui::deserialize<AJson>(sis);
}

AJson AJson::fromBuffer(AByteBufferView buffer) {
    return aui::deserialize<AJson>(AByteBufferInputStream(buffer));
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
