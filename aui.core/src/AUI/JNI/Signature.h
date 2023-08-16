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

#include "Converter.h"

namespace aui::jni {

    template<typename T>
    struct signature;

    template<aui::jni::convertible T>
    struct signature<T> {
        constexpr auto operator()() const noexcept {
            return Converter<std::decay_t<T>>::signature;
        }
    };

    template<typename T>
    inline constexpr auto signature_v = signature<T>{}();

    template<aui::jni::convertible Return, aui::jni::convertible ... Args>
    struct signature<Return(Args...)> {
        constexpr auto operator()() const noexcept {
            if constexpr (sizeof...(Args) > 0) {
                return AStringLiteral<'('>{} + (... + signature_v<Args>) + AStringLiteral<')'>{} + signature_v<Return>;
            } else {
                return AStringLiteral<'(', ')'>{} + signature_v<Return>;
            }
        }
    };
}