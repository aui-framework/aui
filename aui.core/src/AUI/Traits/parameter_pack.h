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

#include <algorithm>
#include <AUI/Traits/concepts.h>

namespace aui {
    namespace impl {
        template<typename First, typename... Up>
        struct first {
            using type = First;
        };
    }

    /**
     * @brief Provides an easy way to iterate over a parameter pack.
     * @ingroup useful_templates
     * @details
     * <table>
     *   <tr>
     *     <td>without</td>
     *     <td>with</td>
     *   </tr>
     *   <tr>
     *     <td>
     *       ```cpp
     *       template<typename Arg, typename... Args> void helperFunc(Arg&& arg, Args&&... args) {
     *          // do something with arg
     *          std::cout << std::forward<Arg>(arg) << std::endl;
     *
     *          if constexpr (sizeof...(args) > 0) {
     *              helperFunc(std::forward<Args>(args)...); // continue iteration
     *          }
     *       }
     *       template<typename... Args> void yourFunc(Args&&... args) {
     *           helperFunc(std::forward<Args>(args)...);
     *       }
     *       ```
     *     </td>
     *     <td>
     *       ```cpp
     *       template<typename... Args> void yourFunc(Args&&... args) {
     *           aui::parameter_pack::for_each([](auto&& i) {
     *               // do something with arg
     *               std::cout << i << " ";
     *           }, std::forward<Args>(args)...);
     *       }
     *       ```
     *     </td>
     *   </tr>

     */
    namespace parameter_pack {
        template<typename Callable, typename... Args>
        static void for_each(Callable&& c, Args&&... args) requires (... && aui::invocable<Callable, Args>) {
            (..., std::invoke(c, std::forward<Args>(args)));
        }

        template<typename... Types>
        using first = typename impl::first<Types...>::type;
    };

    template<typename... Args>
    struct tuple_visitor;

    template<typename... Args>
    struct tuple_visitor<std::tuple<Args...>> {

        /**
         * @brief Visit types (not values) of the tuple, passing each element as single template arguments.
         * @details
         * This helper is especially useful with `aui::reflect::member<>::args` trait:
         * ```cpp
         * void MyClazz::myMethod(int arg1, std::string arg2);
         * aui::tuple_visitor<aui::reflect::member<&MyClazz::myMethod>::args>::for_each_single([]<typename T>() {
         *   cout << AClass<T>::name() << " ";
         * });
         * // -> outputs int, std::string
         * ```
         */
        template<typename Callable>
        static void for_each_single(Callable&& c) {
            (..., c.template operator()<Args>());
        }

        /**
         * @brief Visit types (not values) of the tuple, passing each element as template arguments.
         * @details
         * This helper is especially useful with `aui::reflect::member<>::args` trait:
         * ```cpp
         * void MyClazz::myMethod(int arg1, std::string arg2);
         * aui::tuple_visitor<aui::reflect::member<&MyClazz::myMethod>::args>::for_each_all([]<typename... T>() {
         *   cout << (... << (AClass<T>::name() << " "));
         * });
         * // -> outputs int, std::string
         * ```
         */
        template<typename Callable>
        static auto for_each_all(Callable&& c) {
            return c.template operator()<Args...>();
        }

        /**
         * @brief Visit types (not values) of the tuple, passing each element as single template argument.
         * Unlike for_each_single, produces tuple from callback's return value, respecting argument order.
         * @details
         * This helper is especially useful with `aui::reflect::member<>::args` trait:
         * ```cpp
         * void MyClazz::myMethod(int arg1, std::string arg2);
         * aui::tuple_visitor<aui::reflect::member<&MyClazz::myMethod>::args>::for_each_make_tuple([]<typename T>() {
         *   return readValue<T>();
         * }); // -> std::tuple<int, std::string>
         * ```
         */
        template<typename Callable>
        static auto for_each_make_tuple(Callable&& c) {
            return for_each_make_tuple_impl<Callable&&, Args...>(std::forward<Callable>(c));
        }

    private:
        template<typename Callable, typename First, typename... Remaining>
        static auto for_each_make_tuple_impl(Callable&& c) {
            auto x = c.template operator()<First>();
            return std::tuple_cat(std::make_tuple(std::move(x)),
                                  for_each_make_tuple_impl<Callable&&, Remaining...>(std::forward<Callable>(c)));
        }
        template<typename Callable>
        static auto for_each_make_tuple_impl(Callable&& c) {
            return std::make_tuple();
        }
    };
}