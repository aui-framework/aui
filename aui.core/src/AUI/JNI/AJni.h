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
#include <AUI/Traits/strings.h>
#include <AUI/Common/AException.h>

/**
 * @defgroup jni aui::jni
 * @brief C++/Java bridge.
 * @details Used to make AUI work on Android.
 */


/**
 * @brief Defines getClassName and other useful methods required for Java class definition.
 * @ingroup jni
 * @param name name of cpp class
 * @param path slash-style path to Java class (including name)
 * @details
 * Usage:
 * @code{cpp}
 * namespace com::github::aui::android {
 *     AUI_JNI_CLASS(com/github/aui/android/AUI, AUI) {
 *     public:
 *         // class definition
 *         AUI_JNI_STATIC_METHOD(void, callStaticMethod, ())
 *         AUI_JNI_METHOD(void, CallNonStaticMethod, ())
 *     };
 * }
 * ..
 * com::github::aui::android::AUI::getClassName() -> "com/github/aui/android/AUI"
 * @endcode
 */
#define AUI_JNI_CLASS(path, name) \
class name ## _info: public ::aui::jni::GlobalRef { \
public:                           \
    static constexpr auto JAVA_CLASS_NAME = #path ## _asl;                        \
    [[nodiscard]] static constexpr auto getClassName() noexcept { return #path; } \
    [[nodiscard]] static auto getClass() noexcept { static ::aui::jni::GlobalRef t = ::aui::jni::env()->FindClass(getClassName()); assert(("no such class: " #path, t.asClass() != nullptr)); return t.asClass(); } \
                                      \
};                                \
struct name: public name ## _info

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
 *     AUI_JNI_CLASS(com/github/aui/android/AUI, AUI) {  // required for AUI_JNI_STATIC_METHOD
 *     public:
 *         AUI_JNI_STATIC_METHOD(float, getDpiRatio, ())
 *         AUI_JNI_STATIC_METHOD(void, openUrl, ((const AString&) url))
 *         AUI_JNI_STATIC_METHOD(void, test, ((int) x, (int) y))
 *     };
 * }
 * ..
 * com::github::aui::android::AUI::getClassName() -> "com/github/aui/android/AUI"
 * @endcode
 */
#define AUI_JNI_STATIC_METHOD(ret_t, name, args) \
    static ret_t name (AUI_PP_FOR_EACH(AUI_JNI_INTERNAL_OMIT_BRACES, _, args)) { \
        static_assert(::aui::jni::convertible<ret_t>, "return type is required to be convertible"); \
        auto clazz = getClass();                 \
        auto e = ::aui::jni::env();              \
        const char* signature = ::aui::jni::signature_v<ret_t (AUI_PP_FOR_EACH(AUI_JNI_INTERNAL_OMIT_BRACES, _, args))>; \
        static auto methodId = e->GetStaticMethodID(clazz, #name, signature);    \
        if (methodId == 0) {                          \
            throw AException("no such static jni method: {} {}"_format(#name, signature)); \
        }                                         \
        return ::aui::jni::callStaticMethod<ret_t>(clazz, methodId AUI_PP_FOR_EACH(AUI_JNI_INTERNAL_OMIT_BRACES_CONTENTS, _, args)); \
    }

/**
 * @brief Defines nonstatic method C++ -> Java.
 * @ingroup jni
 * @param ret_t return type.
 * @param name static method's name.
 * @param args arguments wrapped with braces. Types should be also wrapped in braces (see example).
 * @details
 * Usage:
 * @code{cpp}
 * namespace com::github::aui::android {
 *     AUI_JNI_CLASS(com/github/aui/android/AUI, AUI) {  // required for AUI_JNI_STATIC_METHOD
 *     public:
 *         AUI_JNI_METHOD(float, getDpiRatio, ())
 *         AUI_JNI_METHOD(void, openUrl, ((const AString&) url))
 *         AUI_JNI_METHOD(void, test, ((int) x, (int) y))
 *     };
 * }
 * ..
 * com::github::aui::android::AUI::getClassName() -> "com/github/aui/android/AUI"
 * @endcode
 */
#define AUI_JNI_METHOD(ret_t, name, args) \
    ret_t name (AUI_PP_FOR_EACH(AUI_JNI_INTERNAL_OMIT_BRACES, _, args)) { \
        static_assert(::aui::jni::convertible<ret_t>, "return type is required to be convertible"); \
        auto clazz = getClass();                 \
        auto e = ::aui::jni::env();              \
        const char* signature = ::aui::jni::signature_v<ret_t (AUI_PP_FOR_EACH(AUI_JNI_INTERNAL_OMIT_BRACES, _, args))>; \
        static auto methodId = e->GetMethodID(clazz, #name, signature);    \
        if (methodId == 0) {                          \
            throw AException("no such jni method: {} {}"_format(#name, signature)); \
        }                                         \
        return ::aui::jni::callMethod<ret_t>(this->asObject(), methodId AUI_PP_FOR_EACH(AUI_JNI_INTERNAL_OMIT_BRACES_CONTENTS, _, args)); \
    }

#define AUI_JNI_INTERNAL_OMIT_BRACES(i, b, c) AUI_PP_COMMA_IF(i) AUI_PP_IDENTITY c

#define AUI_JNI_INTERNAL_OMIT_BRACES_CONTENTS(i, b, c) , AUI_PP_EMPTY c