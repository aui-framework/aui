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

//
// Created by Alex2772 on 11/19/2021.
//

#pragma once

#include "members.h"

namespace aui {

    template<typename F>
    concept not_overloaded_lambda = requires(F&& f) {
        { &F::operator() };
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
}