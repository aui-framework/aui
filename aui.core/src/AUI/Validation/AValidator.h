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

#pragma once

#include <AUI/Common/AString.h>
#include <type_traits>

namespace aui::valid {
    struct validator {}; // flag class which marks class as a validator

    namespace impl {
        template<typename Validator1, typename Validator2>
        struct And: validator {
            template<typename T>
            bool operator()(T t) const noexcept {
                return Validator1()(t) && Validator2()(t);
            }
        };

        template<typename Validator1, typename Validator2>
        struct Or: validator {
            template<typename T>
            bool operator()(T t) const noexcept {
                return Validator1()(t) || Validator2()(t);
            }
        };
    }

    template<typename T>
    constexpr bool is_validator = std::is_base_of_v<validator, T>;

    template<typename Validator1, typename Validator2, std::enable_if_t<is_validator<Validator1> &&
                                                                        is_validator<Validator2>, bool> = true>
    auto operator||(Validator1 v1, Validator2 v2) noexcept {

        return impl::Or<Validator1, Validator2>();
    }

    template<typename Validator1, typename Validator2, std::enable_if_t<is_validator<Validator1> &&
                                                                        is_validator<Validator2>, bool> = true>
    auto operator&&(Validator1 v1, Validator2 v2) noexcept {

        return impl::And<Validator1, Validator2>();
    }

    /**
     * Is value in range [lower;higher]
     * @tparam T value type (i.e. int)
     */
    template<auto lower, auto higher>
    struct in_range: validator {
        template<typename T>
        bool operator()(T t) const noexcept {
            return t >= lower && t <= higher;
        }
    };

    namespace chars {
        /**
         * Char is latin (a-z, A-Z)
         */
        struct latin: validator {

            bool operator()(int c) const noexcept {
                return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
            }
        };

        /**
         * Char is numeric (0-9)
         */
        struct numeric: validator {
            bool operator()(int c) const noexcept {
                return (c >= '0' && c <= '9');
            }
        };

        /**
         * Char is latin or numeric (a-z, A-Z, 0-9)
         */
        using latin_numeric = decltype(latin() || numeric());
    }

    namespace container {
        /**
         * At least one value is validated by <code>Validator</code>.
         * @tparam Validator individual element validator
         */
        template<typename Validator>
        struct any: validator {
            Validator validator;
            any(Validator validator = {}): validator(std::move(validator)) {}

            template<typename Container>
            bool operator()(const Container& c) const noexcept {
                for (const auto& elem : c) {
                    if (validator(elem)) return true;
                }
                return false;
            }
        };

        /**
         * All values is validated by <code>Validator</code>.
         * @tparam Validator individual element validator
         */
        template<typename Validator>
        struct all: validator {
            Validator validator;
            all(Validator validator = {}): validator(std::move(validator)) {}

            template<typename Container>
            bool operator()(const Container& c) const noexcept {
                for (const auto& elem : c) {
                    if (!Validator()(elem)) return false;
                }
                return true;
            }
        };
    }


    namespace string {

        /**
         * String contains only latin characters (a-z, A-Z)
         */
        using latin = aui::valid::container::all<chars::latin>;

        /**
         * String contains only numeric characters (0-9)
         */
        using numeric = aui::valid::container::all<chars::numeric>;

        /**
         * String contains latin or numeric characters only.
         */
        using latin_numeric = aui::valid::container::all<chars::latin_numeric>;
    }
}
