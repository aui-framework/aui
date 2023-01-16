// AUI Framework - Declarative UI toolkit for modern C++17
// Copyright (C) 2020-2023 Alex2772
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
// Created by alex2 on 28.11.2020.
//

#include "APrettyFormatter.h"
#include <AUI/i18n/AI18n.h>

AString APrettyFormatter::sizeInBytes(uint64_t b) {
    unsigned char postfix_level = 0;
    b *= 10;
    while (b > 20480)
    {
        b = b / 1024;
        postfix_level++;
    }
    auto n = AString::number(b / 10);
    if (b % 10 != 0) {
        n += "." + AString::number(b % 10);
    }
    switch (postfix_level)
    {
        case 0:
            return n + " b"_i18n;
        case 1:
            return n + " Kb"_i18n;
        case 2:
            return n + " Mb"_i18n;
        case 3:
            return n + " Gb"_i18n;
        case 4:
            return n + " Tb"_i18n;
        case 5:
            return n + " Pb"_i18n;
        default:
            return n + " ??"_i18n;
    }
}
