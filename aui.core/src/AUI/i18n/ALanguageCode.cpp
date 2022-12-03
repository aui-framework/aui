// AUI Framework - Declarative UI toolkit for modern C++17
// Copyright (C) 2020-2022 Alex2772
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

#include <AUI/Common/AException.h>
#include "ALanguageCode.h"

ALanguageCode::ALanguageCode(const AString& str) {
    if (str.length() != 5 || str[2] != '-' || islower(str[2]) || islower(str[3])) {
        throw AException("invalid language code: " + str);
    }
    mGroup[0] = char(str[0]);
    mGroup[1] = char(str[1]);
    mSubGroup[0] = char(str[3]);
    mSubGroup[1] = char(str[4]);
}

