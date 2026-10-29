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

#include <AUI/Common/AString.h>

#include <algorithm>
#include <cctype>
#include <string_view>

#ifdef __GNUG__
#include <cstdlib>
#include <memory>
#include <cxxabi.h>
#endif

namespace AReflect {

#if AUI_COMPILER_MSVC
    namespace detail {
        /**
         * @brief Brings a type name of MSVC to the form of GCC and Clang: drops the class keys ("class AButton",
         * "AForEachUI<struct Row>") and spells the anonymous namespace as "(anonymous namespace)".
         */
        inline AString normalizeMsvcTypeName(std::string_view name) {
            static constexpr std::string_view KEYS[] = { "class ", "struct ", "union ", "enum " };
            static constexpr std::string_view ANONYMOUS = "`anonymous namespace'";
            std::string result;
            result.reserve(name.size());
            while (!name.empty()) {
                if (name.starts_with(ANONYMOUS)) {
                    result += "(anonymous namespace)";
                    name.remove_prefix(ANONYMOUS.size());
                    continue;
                }
                const bool wordStart = result.empty() || !(std::isalnum(uint8_t(result.back())) || result.back() == '_');
                if (wordStart) {
                    if (auto key = std::ranges::find_if(KEYS, [&](auto k) { return name.starts_with(k); });
                        key != std::end(KEYS)) {
                        name.remove_prefix(key->size());
                        continue;
                    }
                }
                result += name.front();
                name.remove_prefix(1);
            }
            return result;
        }
    }
#endif

    /**
     * @brief Runtime reflection based on typeid.
     * @ingroup reflection
     * @details
     * The name is the same on all compilers: `AButton`, `ns::Foo<ns::Bar>`, `(anonymous namespace)::Foo`.
     */
    template<typename T>
    AString name(T* v) {
        if (!v) return "nullptr";
#if AUI_COMPILER_MSVC
        return detail::normalizeMsvcTypeName(typeid(*v).name());
#else
        int status;
        auto mangledName = typeid(*v).name();
        auto ptr = abi::__cxa_demangle(mangledName, nullptr, nullptr, &status);
        AString result = ptr;
        free(ptr);
        return result;
#endif
    }
}