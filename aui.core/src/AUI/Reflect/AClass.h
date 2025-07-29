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

#include "AUI/Common/AString.h"


/// [ARROW_ERROR_MESSAGE_EXAMPLE]
/**
 * @brief Compile-time class introspection.
 * @ingroup reflection
 */
template<class T>
class AClass {
public:
    static_assert(!std::is_reference<T>::value, "====================> AClass: attempt to use AClass on a reference.");
/// [ARROW_ERROR_MESSAGE_EXAMPLE]

    static AString name() {
#if AUI_COMPILER_MSVC
        AString s = __FUNCSIG__;
        auto openTag = s.find('<') + 1;
        auto closeTag = s.find('>');
        auto name = s.substr(openTag, closeTag - openTag);
        name = name.substr(name.rfind(' ') + 1);
        if (name.endsWith(" &"))
            name = name.substr(0, name.length() - 2);
        return name;
#elif AUI_COMPILER_CLANG
        #if defined(__PRETTY_FUNCTION__) || defined(__GNUC__) || defined(__clang__)
            AString s = __PRETTY_FUNCTION__;
        #elif defined(__FUNCSIG__)
            AString s = __FUNCSIG__;
        #else
            AString s = __FUNCTION__;
        #endif
        auto b = s.find("=") + 1;
        auto e = s.find("&", b);
        e = std::min(s.find("]", b), e);
        auto result = s.substr(b, e - b);
        result = result.trim();
        return result;
#else
        #if defined(__PRETTY_FUNCTION__) || defined(__GNUC__) || defined(__clang__)
            AString s = __PRETTY_FUNCTION__;
        #elif defined(__FUNCSIG__)
            AString s = __FUNCSIG__;
        #else
            AString s = __FUNCTION__;
        #endif
        auto b = s.find("with T = ") + 9;
        return { s.begin() + b, s.end() - 1 };
#endif
    }

    static AString nameWithoutNamespace() {
        auto s = name();
        auto p = s.rfind("::");
        if (p != AString::NPOS) {
            return {s.begin() + p + 2, s.end()};
        }
        return s;
    }

    static AString toString(const T& t) {
        return "<object of type " + name() + ">";
    }
};

template<>
inline AString AClass<AString>::toString(const AString& t) {
    return "\"" + t + "\"";
}

template<>
inline AString AClass<int>::toString(const int& t) {
    return AString::number(t);
}