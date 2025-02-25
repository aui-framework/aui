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

#include <functional>

namespace aui::reflect {

// based on ideas found in Boost.PFR.
// credits: Antony Polukhin

namespace detail {
template <class T, class F, std::size_t... I>
void for_each_field_dispatcher(T& t, F&& f, std::index_sequence<I...>) {
    static_assert(
        !std::is_union<T>::value,
        "====================> aui::reflect: attempt to reflect on a union."
    );
    std::forward<F>(f)(
        detail::tie_as_tuple(t)
    );
}
}

/**
 * @brief Calls `callback` for each field of a `value`.
 * @ingroup reflection
 */
template<class T, typename F>
void for_each_field(T&& value, F&& callback) {

}
}