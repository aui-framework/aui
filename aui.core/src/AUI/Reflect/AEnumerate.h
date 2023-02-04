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

//
// Created by alex2 on 6/24/2021.
//

#pragma once

#include <type_traits>
#include <AUI/Util/EnumUtil.h>
#include <AUI/Common/AString.h>
#include <AUI/Common/AMap.h>
#include <AUI/Traits/types.h>
#include <fmt/format.h>

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

    template<enum_t value>
    static AString valueName() {
#ifdef _MSC_VER
        AString s = __FUNCSIG__;
        AString::iterator end = s.begin() + s.rfind('>');
        AString::iterator begin = (std::find_if(std::make_reverse_iterator(end), s.rend(), [](wchar_t c) {
            return c == ':' || c == '<';
        })).base();

        AString result(begin, end);
#else
        AString s = __PRETTY_FUNCTION__;
        auto end = s.rfind(';');
        size_t begin;
        begin = s.rfind("value =", end);
        if (begin == AString::NPOS) {
            begin = s.rfind('[', end) + 1;
        } else {
            begin += 8;
        }
        AString result = {s.begin() + begin, s.begin() + end};


        for (size_t p; (p = result.find("::")) != AString::NPOS;) {
            result = result.substr(p + 2);
        }

#endif
        return result;
    }

    template<enum_t... values>
    static const AMap<AString, enum_t>& mapValueByName() {
        static AMap<AString, enum_t> map = {
            {valueName<values>(), values}...
        };
        return map;
    }

    template<enum_t... values>
    static const AMap<enum_t, AString, enum_less>& mapNameByValue() {
        static AMap<enum_t, AString, enum_less> map = {
            {values, valueName<values>() }...
        };
        return map;
    }

    static const AMap<AString, enum_t>& all();
    static const AMap<enum_t, AString, enum_less>& names();

    static enum_t byName(const AString& name) {
        if (auto c = all().contains(name.uppercase())) {
            return c->second;
        }
        return (enum_t)0;
    }

private:
    template<enum_t... values>
    static const AMap<AString, enum_t>& mapValueByNameImpl(const Values<values...>& v) {
        return mapValueByName<values...>();
    }
    template<enum_t... values>
    static const AMap<enum_t, AString, enum_less>& mapNameByValueImpl(const Values<values...>& v) {
        return mapNameByValue<values...>();
    }
};

template<typename enum_t>
struct AEnumerateAllValues;

template<typename enum_t>
const AMap<AString, enum_t>& AEnumerate<enum_t>::all() {
    static_assert(aui::is_complete<AEnumerateAllValues<enum_t>>, "AUI_ENUM_VALUES is not defined for this enum type");
    auto v = AEnumerateAllValues<enum_t>::get();

    return mapValueByNameImpl(v);
}

template<typename enum_t>
const AMap<enum_t, AString, typename AEnumerate<enum_t>::enum_less>& AEnumerate<enum_t>::names() {
    static_assert(aui::is_complete<AEnumerateAllValues<enum_t>>, "AUI_ENUM_VALUES is not defined for this enum type");
    auto v = AEnumerateAllValues<enum_t>::get();

    return mapNameByValueImpl(v);
}

#define AUI_ENUM_VALUES(enum_t, ...) template<> \
struct AEnumerateAllValues<enum_t>{         \
    static inline constexpr AEnumerate<enum_t>::Values<__VA_ARGS__> get() {return {}; } \
};                                         \
inline std::ostream& operator<<(std::ostream& o, enum_t v) { return o << AEnumerate<enum_t>::names()[v]; } \
namespace std { inline AString to_wstring(enum_t v) { return AEnumerate<enum_t>::names()[v]; } }

template <typename T> struct fmt::formatter<T, char, std::enable_if_t<aui::is_complete<AEnumerateAllValues<T>>>>: formatter<std::string> {
    // parse is inherited from formatter<string_view>.
    template <typename FormatContext>
    auto format(T c, FormatContext& ctx) const {
        return formatter<string_view>::format(AEnumerate<T>::names()[c].toStdString(), ctx);
    }
};
