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
// Created by Alex2772 on 12/5/2021.
//

#pragma once

#include <AUI/Util/AMetric.h>


template<int I>
struct ViewAssertionSize {
    int value;
    explicit ViewAssertionSize(const AMetric& value): value(value.getValuePx()) {}

    bool operator()(const _<AView>& v) {
        return v->getSize()[I] == value;
    }
};

template<int I>
struct ViewAssertionSizeSame {
    AOptional<int> value;

    bool operator()(const _<AView>& v) {
        if (!value) {
            value = v->getSize()[I];
            return true;
        }
        return v->getSize()[I] == *value;
    }
};

template<int I>
struct ViewAssertionSizeIsMinimal {
    AOptional<int> value;

    bool operator()(const _<AView>& v) {
        return v->getSize()[I] == v->getMinimumSize()[I];
    }
};

using width = ViewAssertionSize<0>;
using height = ViewAssertionSize<1>;

using widthIsMinimal = ViewAssertionSizeIsMinimal<0>;
using heightIsMinimal = ViewAssertionSizeIsMinimal<1>;

using sameWidth = ViewAssertionSizeSame<0>;
using sameHeight = ViewAssertionSizeSame<1>;