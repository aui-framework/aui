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

//
// Created by alex2 on 6/24/2021.
//

#pragma once

#include <array>
#include <type_traits>
#include <AUI/Common/AString.h>
#include <AUI/Common/AMap.h>
#include <AUI/Traits/types.h>
#include <fmt/format.h>


//NOLINTBEGIN(modernize-*,cppcoreguidelines-macro-usage,bugprone-macro-parentheses)

namespace aui::enumerate::basic {
    template<typename enum_t>
    struct ValueToName {
        template<enum_t value>
        static AString name() {
#if AUI_COMPILER_MSVC
            AString s = __FUNCSIG__;
        AString::iterator end = s.begin() + s.rfind('>');
        AString::iterator begin = (std::find_if(std::make_reverse_iterator(end), s.rend(), [](wchar_t c) {
            return c == ':' || c == '<';
        })).base();

        AString result(begin, end);
#else
        #if defined(__PRETTY_FUNCTION__) || defined(__GNUC__) || defined(__clang__)
            AString s = __PRETTY_FUNCTION__;
        #elif defined(__FUNCSIG__)
            AString s = __FUNCSIG__;
        #else
            AString s = __FUNCTION__;
        #endif
#if AUI_COMPILER_CLANG
            auto end = s.rfind(']');
#else
            auto end = s.rfind(';');
#endif
            size_t begin = s.rfind("value =", end);
            if (begin == AString::NPOS) {
                begin = s.rfind('[', end) + 1;
            } else {
                begin += 8;
            }
            AString result = {s.bytes().begin() + begin, s.bytes().begin() + end};


            for (size_t p; (p = result.find("::")) != AString::NPOS;) {
                result = result.substr(p + 2);
            }

#endif
            return result;
        }
    };
};

namespace aui::enumerate {
    template<typename enum_t>
    struct ValueToName: basic::ValueToName<enum_t> {};
}


/**
 * @brief Enum trait to transform enum to name, name to enum, list all enums and vise versa.
 * @ingroup reflection
 * @see AUI_ENUM_VALUES
 */
template<typename enum_t>
class AEnumerate {
private:
    struct enum_less {
        constexpr bool operator()(enum_t l, enum_t r) const {
            return static_cast<std::underlying_type_t<enum_t>>(l) < static_cast<std::underlying_type_t<enum_t>>(r);
        }
    };

public:
    static_assert(std::is_enum_v<enum_t>, "AEnumerate accepts only enums");

    template<enum_t... values>
    struct Values {

    };

    /**
     * @brief Maps compile-time specified enum value to name.
     * @details
     * Use `names()[enumValue]` for mapping runtime values.
     *
     * This function's behaviour can be reimplemented by aui::enumerate::ValueToName trait.
     */
    template<enum_t value>
    static AString valueName() {
        return aui::enumerate::ValueToName<enum_t>::template name<value>();
    }

    /**
     * @brief Get runtime name to enum value mapping.
     * @see byName
     */
    static const AMap<AString, enum_t>& nameToValueMap();

    /**
     * @brief Map runtime enum value to name.
     */
    static const AMap<enum_t, AString, enum_less>& valueToNameMap();

    /**
     * @brief Map runtime name to enum value. Transforms name to uppercase as a fallback. Throws an exception if no such value.
     */
    static enum_t byName(const AString& name) {
        if (auto c = nameToValueMap().contains(name)) {
            return c->second;
        }
        if (auto c = nameToValueMap().contains(name.uppercase())) {
            return c->second;
        }
        throw AException("unknown enum value: \"{}\""_format(name));
    }

    /**
     * @brief Map runtime enum value to name. Throws an exception if no such value.
     */
    static const AString& toName(enum_t value) {
        if (auto c = valueToNameMap().contains(value)) {
            return c->second;
        }
        throw AException("unknown enum value: \"(int){}\""_format((std::underlying_type_t<enum_t>)value));
    }

private:
    template<enum_t... values>
    static const AMap<AString, enum_t>& mapValueByName(const Values<values...>& v) {
        static AMap<AString, enum_t> map = {
            {valueName<values>(), values}...
        };
        return map;
    }
    template<enum_t... values>
    static const AMap<enum_t, AString, enum_less>& mapNameByValue(const Values<values...>& v) {
        static AMap<enum_t, AString, enum_less> map = {
            {values, valueName<values>() }...
        };
        return map;
    }
};

template<typename enum_t>
struct AEnumerateAllValues;

template<typename enum_t>
const AMap<AString, enum_t>& AEnumerate<enum_t>::nameToValueMap() {
    static_assert(aui::is_complete<AEnumerateAllValues<enum_t>>, "AUI_ENUM_VALUES is not defined for this enum type");
    auto v = AEnumerateAllValues<enum_t>::get();

    return mapValueByName(v);
}

namespace aui::enumerate {
    /**
     * @brief constexpr std::array of all possible enum values is the order they've been passed to AUI_ENUM_VALUES.
     * @ingroup core
     * @see AUI_ENUM_VALUES
     */
    template<typename enum_t> requires aui::is_complete<AEnumerateAllValues<enum_t>>
    inline constexpr auto ALL_VALUES = []<auto... values>(typename AEnumerate<enum_t>::template Values<values...>) {
        constexpr enum_t ITEMS[] = {values...};
        return std::to_array(ITEMS);
    }(AEnumerateAllValues<enum_t>::get());
}

template<typename enum_t>
const AMap<enum_t, AString, typename AEnumerate<enum_t>::enum_less>& AEnumerate<enum_t>::valueToNameMap() {
    static_assert(aui::is_complete<AEnumerateAllValues<enum_t>>, "AUI_ENUM_VALUES is not defined for this enum type");
    auto v = AEnumerateAllValues<enum_t>::get();

    return mapNameByValue(v);
}

/**
 * @brief Defines all enum values for AEnumerate.
 * @ingroup useful_macros
 * @details
 * Defines all enum values to by used by AEnumerate.
 * ```cpp
 * enum class ATextOverflow {
 *     NONE,
 *     ELLIPSIS,
 *     CLIP
 * };
 * AUI_ENUM_VALUES(ATextOverflow,
 *                 ATextOverflow::ELLIPSIS,
 *                 ATextOverflow::CLIP)
 *
 * // AEnumerate<ATextOverflow>::toName(ATextOverflow::CLIP) -> "CLIP"
 * ```
 */
#define AUI_ENUM_VALUES(enum_t, ...) template<> \
struct AEnumerateAllValues<enum_t>{         \
    static inline constexpr AEnumerate<enum_t>::Values<__VA_ARGS__> get() {return {}; } \
};                                         \
namespace std { inline AString to_wstring(enum_t v) { return AEnumerate<enum_t>::valueToNameMap().optional(v).valueOr("<unknown enum value {}>"_format(int(v))); } } \
inline std::ostream& operator<<(std::ostream& o, enum_t v) { return o << std::to_wstring(v); }

template <typename T> struct fmt::formatter<T, char, std::enable_if_t<aui::is_complete<AEnumerateAllValues<T>>>>: formatter<std::string> {
    // parse is inherited from formatter<string_view>.
    template <typename FormatContext>
    auto format(T c, FormatContext& ctx) const {
        return formatter<string_view>::format(AEnumerate<T>::valueToNameMap()[c].toStdString(), ctx);
    }
};

/**
 * @brief Make a bitfield-style enum class.
 * @ingroup core
 */
#define AUI_ENUM_FLAG(name) enum class name: int; \
                            constexpr inline name operator|(name a, name b) {return static_cast<name>(static_cast<int>(a) | static_cast<int>(b));} \
                            constexpr inline name operator&(name a, name b) {return static_cast<name>(static_cast<int>(a) & static_cast<int>(b));} \
                            constexpr inline name operator^(name a, name b) {return static_cast<name>(static_cast<int>(a) ^ static_cast<int>(b));} \
                            constexpr inline name operator|=(name& a, name b) {return a = static_cast<name>(static_cast<int>(a) | static_cast<int>(b));} \
                            constexpr inline name operator&=(name& a, name b) {return a = static_cast<name>(static_cast<int>(a) & static_cast<int>(b));} \
                            constexpr inline name operator^=(name& a, name b) {return a = static_cast<name>(static_cast<int>(a) ^ static_cast<int>(b));} \
                            constexpr inline name operator~(const name& a) {return static_cast<name>(~static_cast<int>(a));} \
                            constexpr inline bool operator!(const name& a) {return a == static_cast<name>(0);}                                     \
                                                                   \
                            constexpr inline bool operator&&(const name& a, bool v) {return static_cast<int>(a) && v;}                                     \
                            constexpr inline bool operator||(const name& a, bool v) {return static_cast<int>(a) || v;}                                     \
\
                            enum class name: int

#define AUI_ENUM_INT(name) enum class name: int; \
                           constexpr inline bool operator<(name a, name b) {return static_cast<int>(a) < static_cast<int>(b);} \
                           constexpr inline bool operator>(name a, name b) {return static_cast<int>(a) > static_cast<int>(b);} \
                           enum class name: int

//NOLINTEND(modernize-*,cppcoreguidelines-macro-usage,bugprone-macro-parentheses)