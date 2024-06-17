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
#include "TypedMethods.h"
#include "Signature.h"

namespace aui::jni {
    /**
     * @brief Calls static method.
     * @tparam Return return type.
     * @tparam Args argument types.
     * @return result
     */
    template<convertible Return = void, convertible... Args>
    Return callStaticMethod(jclass clazz, jmethodID methodId, const Args&... args) {
        auto e = env();
        if constexpr (std::is_void_v<Return>) {
            (e->*TypedMethods<Return>::CallStaticMethod)(clazz, methodId, toJni(args)...);
        } else {
            return fromJni<Return>((e->*TypedMethods<java_t_from_cpp_t<Return>>::CallStaticMethod)(clazz, methodId, toJni(args)...));
        }
    }

    /**
     * @brief Calls nonstatic method.
     * @tparam Return return type.
     * @tparam Args argument types.
     * @return result
     */
    template<convertible Return = void, convertible... Args>
    Return callMethod(jobject thiz, jmethodID methodId, const Args&... args) {
        auto e = env();
        if constexpr (std::is_void_v<Return>) {
            (e->*TypedMethods<Return>::CallMethod)(thiz, methodId, toJni(args)...);
        } else {
            return fromJni<Return>((e->*TypedMethods<java_t_from_cpp_t<Return>>::CallMethod)(thiz, methodId, toJni(args)...));
        }
    }
}