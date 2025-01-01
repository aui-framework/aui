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

//
// Created by Alex2772 on 11/19/2021.
//

#pragma once

#include "members.h"

namespace aui {

    template<typename F>
    concept not_overloaded_lambda = requires(F&& f) {
        // we can't 100% guarantee that T is actual lambda, but C++ lambdas have following traits:
        std::is_class_v<F>;
        { &std::decay_t<F>::operator() };
    };

    static_assert(not_overloaded_lambda<decltype([]{})>, "aui::not_overloaded_lambda failed");
    static_assert(not_overloaded_lambda<decltype([](int v){})>, "aui::not_overloaded_lambda failed");

    /**
     * That `<code>overloaded</code>` trick
     */
    template<typename... Lambdas>
    struct lambda_overloaded : Lambdas... {
        using Lambdas::operator()...;
    };

    // deduction guide
    template<typename... Lambdas>
    lambda_overloaded(Lambdas...) -> lambda_overloaded<Lambdas...>;

    template<typename Return, typename... Args>
    struct function_info;

    template<typename Return, typename... Args>
    struct function_info<Return(Args...)> {
        using return_t = Return;
        using args = std::tuple<Args...>;
    };

    template<not_overloaded_lambda Lambda>
    using lambda_info = member<decltype(&Lambda::operator())>;

    /**
     * @brief Function object type whose `operator()` returns its argument unchanged.
     * @details
     * Implementation of std::identity.
     */
    struct identity
    {
        template<typename T>
        [[nodiscard]]
        constexpr T&& operator()(T&& t) const noexcept { return std::forward<T>(t); }
    };
}