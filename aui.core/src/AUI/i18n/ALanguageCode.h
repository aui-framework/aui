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

//
// Created by alex2 on 07.11.2020.
//

#pragma once


#include <AUI/Common/AString.h>

/**
 * @brief Represents a language code in ISO 639-1, for example, en-US or ru-RU
 * @ingroup core
 */
class API_AUI_CORE ALanguageCode {
private:
    char mGroup[2];
    char mSubGroup[2];
public:
    ALanguageCode(const AString& str);
    ALanguageCode(const char* str): ALanguageCode(AString(str)) {}

    [[nodiscard]]
    AString toString() const {
        return AString(mGroup[0]) + mGroup[1] + "-" + mSubGroup[0] + mSubGroup[1];
    }

    [[nodiscard]]
    AString getGroup() const {
        return AString(mGroup, mGroup + 2);
    }

    [[nodiscard]]
    AString getSubGroup() const {
        return AString(mSubGroup, mSubGroup + 2);
    }
};


