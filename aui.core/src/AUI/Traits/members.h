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

#include <algorithm>
#include <tuple>

namespace aui {
    template<typename Type>
    struct member;

    template<typename Type, typename Clazz>
    struct member<Type(Clazz::*)> {
        using type = Type;
        using clazz = Clazz;
    };

    template<typename Type, typename Clazz, typename... Args>
    struct member<Type(Clazz::*)(Args...) const> {
        using return_t = Type;
        using clazz = Clazz;
        /**
         * @brief Argument types.
         * @see aui::tuple_visitor
         */
        using args = std::tuple<Args...>;
    };

    template<typename Type, typename Clazz, typename... Args>
    struct member<Type(Clazz::*)(Args...)> {
        using return_t = Type;
        using clazz = Clazz;
        /**
         * @brief Argument types.
         * @see aui::tuple_visitor
         */
        using args = std::tuple<Args...>;
    };

    template<typename Type, typename Clazz, typename... Args>
    struct member<Type(Clazz::*)(Args...) noexcept> {
        using return_t = Type;
        using clazz = Clazz;
        /**
         * @brief Argument types.
         * @see aui::tuple_visitor
         */
        using args = std::tuple<Args...>;
    };

    template<typename Type, typename Clazz, typename... Args>
    struct member<Type(Clazz::*)(Args...) const noexcept> {
        using return_t = Type;
        using clazz = Clazz;
        /**
         * @brief Argument types.
         * @see aui::tuple_visitor
         */
        using args = std::tuple<Args...>;
    };
}