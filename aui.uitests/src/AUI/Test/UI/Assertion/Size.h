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
        auto viewSize = v->getSize()[I];
        EXPECT_EQ(viewSize, value);
        return viewSize == value;
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