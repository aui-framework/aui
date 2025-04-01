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

#include <functional>
#include <AUI/Reflect/detail/gen.h>
#include <AUI/Reflect/detail/fields_count.h>

namespace aui::reflect::detail {

// based on ideas found in Boost.PFR.
// credits: Antony Polukhin


template <class Clazz>
constexpr auto tie_as_tuple(Clazz& clazz) noexcept {
    static_assert(
        !std::is_union<Clazz>::value,
        "====================> aui::reflect: attempt to reflect on a union."
    );
    typedef std::integral_constant<std::size_t, detail::fields_count<Clazz>()> fieldsCountTag;
    return detail::tie_as_tuple(clazz, fieldsCountTag{});
}

template <class Clazz, class F, std::size_t... I>
constexpr void for_each_field_dispatcher(Clazz& clazz, F&& callback, std::index_sequence<I...>) {
    static_assert(
        !std::is_union<Clazz>::value,
        "====================> aui::reflect: attempt to reflect on a union."
    );
    std::forward<F>(callback)(
        detail::tie_as_tuple(clazz)
    );
}

}