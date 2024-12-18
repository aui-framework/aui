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

#pragma once

#include <type_traits>

namespace aui {
    template<typename boolean, typename OnTrue, typename OnFalse>
    struct ternary {};

    template<typename OnTrue, typename OnFalse>
    struct ternary<std::true_type, OnTrue, OnFalse> {
        using value = OnTrue;
    };

    template<typename OnTrue, typename OnFalse>
    struct ternary<std::false_type, OnTrue, OnFalse> {
        using value = OnFalse;
    };

    template<typename boolean, typename OnTrue, typename OnFalse>
    using ternary_v = typename ternary<boolean, OnTrue, OnFalse>::value;
}