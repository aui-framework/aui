// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2024 Alex2772 and Contributors
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
