// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2023 Alex2772
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

#include <concepts>

namespace aui {
    template<typename F, typename From, typename To>
    concept mapper = requires(F&& f, From&& from) {
        { std::invoke(f, std::forward<From>(from)) } -> std::convertible_to<To>;
    };

    template<typename F, typename ProducedObject>
    concept factory = requires(F&& f) {
        { std::invoke(f) } -> std::convertible_to<ProducedObject>;
    };
}