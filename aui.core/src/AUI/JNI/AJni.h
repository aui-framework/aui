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

#include "Globals.h"
#include "GlobalRef.h"
#include "Converter.h"
#include "Signature.h"
#include "Env.h"
#include <AUI/Util/APreprocessor.h>

/**
 * @defgroup jni aui::jni
 * @brief C++/Java bridge.
 * @details Used to work on Android.
 */


/**
 * @brief Defines getClassName and other useful methods required for Java class definition.
 * @ingroup jni
 * @param name path to Java class
 * @details
 * Usage:
 * @code{cpp}
 * namespace com::github::aui::android {
 *     class AUI {
 *     public:
 *         AUI_JNI_CLASS(com/github/aui/android/AUI)
 *     };
 * }
 * ..
 * com::github::aui::android::AUI::getClassName() -> "com/github/aui/android/AUI"
 * @endcode
 */
#define AUI_JNI_CLASS(name) \
    [[nodiscard]] static constexpr auto getClassName() noexcept { return #name; } \
    [[nodiscard]] static auto getClass() noexcept { static ::aui::jni::GlobalRef t = ::aui::jni::env()->FindClass(getClassName()); assert(("no such class: " #name, t.asClass() != nullptr)); return t.asClass(); }

/**
 * @brief Defines static method C++ -> Java.
 * @ingroup jni
 * @param ret_t return type.
 * @param name static method's name.
 * @param args arguments wrapped with braces. Types should be also wrapped in braces (see example).
 * @details
 * Usage:
 * @code{cpp}
 * namespace com::github::aui::android {
 *     class AUI {
 *     public:
 *         AUI_JNI_CLASS(com/github/aui/android/AUI)
 *     };
 * }
 * ..
 * com::github::aui::android::AUI::getClassName() -> "com/github/aui/android/AUI"
 * @endcode
 */
#define AUI_JNI_STATIC_METHOD(ret_t, name, args) \
    [[nodiscard]] static ret_t name (AUI_PP_FOR_EACH(AUI_JNI_INTERNAL_OMIT_BRACES, _, args)) { \
        auto clazz = getClass();                 \
        auto e = ::aui::jni::env();              \
        const char* signature = ::aui::jni::signature_v<ret_t (AUI_PP_FOR_EACH(AUI_JNI_INTERNAL_OMIT_BRACES, _, args))>; \
        static auto methodId = e->GetStaticMethodID(clazz, #name, signature); \
        return ::aui::jni::callStaticMethod<ret_t>(clazz, methodId AUI_PP_FOR_EACH(AUI_JNI_INTERNAL_OMIT_BRACES_CONTENTS, _, args)); \
    }

#define AUI_JNI_INTERNAL_OMIT_BRACES(a, b, c) AUI_PP_IDENTITY c

#define AUI_JNI_INTERNAL_OMIT_BRACES_CONTENTS(i, b, c) , AUI_PP_EMPTY c