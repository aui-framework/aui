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
// Created by Alex2772 on 12/5/2021.
//

#pragma once

#include <AUI/Util/AMetric.h>
#include <AUI/Common/ASide.h>
#include "Traits.h"

struct ViewAssertionParentOf {
    using IGNORE_VISIBILITY = std::true_type;

    _<AView> checkedParent;

    explicit ViewAssertionParentOf(_<AView> checkedParent) : checkedParent(std::move(checkedParent)) {}

    bool operator()(const _<AView>& v) {
        return checkedParent->hasIndirectParent(v);
    }
};


using isParentOf = ViewAssertionParentOf;