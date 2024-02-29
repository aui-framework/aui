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
// Created by Alex2772 on 11/19/2021.
//

#pragma once


#include <AUI/Reflect/AEnumerate.h>

AUI_ENUM_FLAG(Repeat) {
        NONE = 0,
        X_Y = 0b11,
        X = 0b01,
        Y = 0b10,
};

AUI_ENUM_VALUES(Repeat, Repeat::NONE, Repeat::X, Repeat::Y, Repeat::X_Y)