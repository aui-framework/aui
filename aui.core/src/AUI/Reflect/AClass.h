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

    static constexpr AStringView name() {
#if AUI_COMPILER_MSVC || defined(__RESHARPER__)
        AStringView s = __FUNCSIG__;
        auto openTag = s.find('<') + 1;
        auto closeTag = s.find('>');
        auto name = s.substr(openTag, closeTag - openTag);
        name = name.bytes().substr(name.bytes().rfind(' ') + 1);
        if (name.endsWith(" &"))
            name = name.bytes().substr(0, name.bytes().size() - 2);
        return name;
#elif AUI_COMPILER_CLANG || defined(_CLANGD)
        #if defined(__PRETTY_FUNCTION__) || defined(__GNUC__) || defined(__clang__)  || defined(_CLANGD)
        AStringView s = __PRETTY_FUNCTION__;
        #elif defined(__FUNCSIG__)
        AStringView s = __FUNCSIG__;
        #else
        AStringView s = __FUNCTION__;
        #endif
        auto b = s.find("=") + 1;
        auto e = s.find("&", b);
        e = std::min(s.find("]", b), e);
        auto result = AStringView(s.bytes().substr(b, e - b));
        result = result.trim();
        return AStringView(result);
#else
        #if defined(__PRETTY_FUNCTION__) || defined(__GNUC__) || defined(__clang__)
            AStringView s = __PRETTY_FUNCTION__;
        #elif defined(__FUNCSIG__)
            AStringView s = __FUNCSIG__;
        #else
            AStringView s = __FUNCTION__;
        #endif
        auto b = s.find("with T = ") + 9;
        return { s.bytes().begin() + b, s.bytes().end() - 1 };
#endif
    }

    static constexpr AStringView nameWithoutNamespace() {
        auto s = name();
        auto p = s.rfind("::");
        if (p != AString::NPOS) {
            return s.substr(p + 2);
        }
        return s;
    }
};

static_assert(AClass<AString>::name() == "AString");
