#  AUI Framework - Declarative UI toolkit for modern C++20
#  Copyright (C) 2020-2025 Alex2772 and Contributors
#
#  SPDX-License-Identifier: MPL-2.0
#
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at http://mozilla.org/MPL/2.0/.

from pathlib import Path

MAX_FIELD_COUNT = 100

PROLOGUE = """/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2025 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

// based on the ideas found in boost.pfr.
// credits: Antony Polukhin, Denis Mikhailov

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////                                                                     ///////////////////////////
////////////////////////  THIS HEADER IS AUTOGENERATED WITH aui.core/misc/gen_reflection.py  ///////////////////////////
////////////////////////                                                                     ///////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <tuple>
#include <type_traits>

namespace aui::reflect::detail {

template <class... Args>
constexpr auto make_tuple_of_references(Args&&... args) noexcept {
    return std::make_tuple<Args&...>(args...);
}

template<typename T, typename Arg>
constexpr decltype(auto) add_cv(Arg& arg) noexcept {
    if constexpr (std::is_const<T>::value && std::is_volatile<T>::value) {
        return const_cast<const volatile Arg&>(arg);
    }
    else if constexpr (std::is_const<T>::value) {
        return const_cast<const Arg&>(arg);
    }
    else if constexpr (std::is_volatile<T>::value) {
        return const_cast<volatile Arg&>(arg);
    }
    else {
        return const_cast<Arg&>(arg);
    }
}

// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=78939
template<typename T, typename Sb, typename Arg>
constexpr decltype(auto) workaround_cast(Arg& arg) noexcept {
    using output_arg_t = std::conditional_t<!std::is_reference<Sb>(), decltype(detail::add_cv<T>(arg)), Sb>;
    return const_cast<output_arg_t>(arg);
}

template<class Clazz>
constexpr auto tie_as_tuple(Clazz& /* clazz */, std::integral_constant<std::size_t, 0>) noexcept {
  return std::tuple<>{};
}

template <class Clazz>
constexpr auto tie_as_tuple(Clazz& clazz, std::integral_constant<std::size_t, 1>, std::enable_if_t<std::is_class< std::remove_cv_t<Clazz> >::value>* = nullptr) noexcept {
  auto& [a] = const_cast<std::remove_cv_t<Clazz>&>(clazz); // ====================> aui::reflect: Clazz is not a SimpleAggregate.
  return detail::make_tuple_of_references(detail::workaround_cast<Clazz, decltype(a)>(a));
}


template <class Clazz>
constexpr auto tie_as_tuple(Clazz& clazz, std::integral_constant<std::size_t, 1>, std::enable_if_t<!std::is_class< std::remove_cv_t<Clazz> >::value>* = nullptr) noexcept {
  return detail::make_tuple_of_references(clazz);
}

"""

EPILOGUE = """
template <class Clazz, std::size_t I>
constexpr void tie_as_tuple(Clazz& /* clazz */, std::integral_constant<std::size_t, I>) noexcept {
  static_assert(sizeof(Clazz) && false,
                "====================> aui::reflect: too many fields in a structure Clazz. Please regenerate aui.core/src/AUI/Reflect/detail/gen.h for an appropriate count of fields.");
}

} // namespace aui::reflect::detail
"""

def make_cast(i):
    return f"detail::workaround_cast<Clazz, decltype({i})>({i})"

if __name__ == "__main__":
    target = Path(__file__).parent.parent / "src" / "AUI" / "Reflect" / "detail" / "gen.h"
    with open(target, 'w') as f:
        f.write(PROLOGUE)

        for i in range(1, MAX_FIELD_COUNT):
            indices = [f"f{i}" for i in range(0, i)]
            f.write(f"template<typename Clazz>\n")
            f.write(f"constexpr auto tie_as_tuple(Clazz& clazz, std::integral_constant<std::size_t, {i}>) noexcept {{\n")
            f.write(f"    auto& [\n")
            f.write(f"    {', '.join(indices)}\n")
            f.write(f"    ] = const_cast<std::remove_cv_t<Clazz>&>(clazz); // ====================> aui::reflect: Clazz is not a SimpleAggregate.\n")
            f.write(f"    return detail::make_tuple_of_references({ ', '.join([make_cast(i) for i in indices]) });\n")
            f.write(f"}}\n")

        f.write(EPILOGUE)
