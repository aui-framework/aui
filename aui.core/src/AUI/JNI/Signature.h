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