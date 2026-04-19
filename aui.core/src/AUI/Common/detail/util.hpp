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

#pragma once

template <typename OutT, typename InT>
constexpr const OutT* pointer_cast(const InT* ptr) {
    static_assert(sizeof(InT) == sizeof(OutT), "Size mismatch");
    static_assert(alignof(InT) == alignof(OutT), "Alignment mismatch");

    union Converter {
        const InT* from;
        const OutT* to;

        constexpr Converter(const InT* p) : from(p) {}
    };

    return Converter(ptr).to;
}

template <typename OutT, typename InT>
constexpr const OutT* pointer_cast(InT* ptr) {
    static_assert(sizeof(InT) == sizeof(OutT), "Size mismatch");
    static_assert(alignof(InT) == alignof(OutT), "Alignment mismatch");

    union Converter {
        InT* from;
        OutT* to;

        constexpr Converter(InT* p) : from(p) {}
    };

    return Converter(ptr).to;
}
