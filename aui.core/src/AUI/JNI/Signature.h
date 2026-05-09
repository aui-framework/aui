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

    template<typename Return, typename ... Args>
    struct signature<Return(Args...)> {
        constexpr auto operator()() const noexcept {
            checkIsConvertible<Return>();
            (checkIsConvertible<Args>(), ...);
            if constexpr (sizeof...(Args) > 0) {
                return AStringLiteral<'('>{} + (... + signature_v<Args>) + AStringLiteral<')'>{} + signature_v<Return>;
            } else {
                return AStringLiteral<'(', ')'>{} + signature_v<Return>;
            }
        }

    private:
        template<typename Arg>
        static constexpr auto checkIsConvertible() {
            static_assert(aui::jni::convertible<Arg>, "All types within function signature must conform aui::jni::convertible");
        }
    };
}
