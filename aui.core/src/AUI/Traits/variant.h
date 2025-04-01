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

#include <cstdint>
#include <type_traits>

namespace aui::variant {

/**
 * @brief Determines type index inside of std::variant.
 * @ingroup useful_templates
 */
template<typename Variant, typename Type>
struct index_of {
private:
    template<std::size_t i, std::size_t... next>
    static constexpr std::size_t value_impl2(std::index_sequence<i, next...>) {
        if constexpr (std::is_same_v<std::variant_alternative_t<i, Variant>, Type>) {
            return i;
        } else {
            return value_impl2(std::index_sequence<next...>{});
        }
    }

    static constexpr std::size_t value_impl() {
        return value_impl2(std::make_index_sequence<std::variant_size_v<Variant>>());
    }
public:
    static constexpr std::size_t value = value_impl();
};
}