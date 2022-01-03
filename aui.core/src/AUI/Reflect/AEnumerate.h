//
// Created by alex2 on 6/24/2021.
//

#pragma once

#include <type_traits>
#include <AUI/Util/EnumUtil.h>
#include <AUI/Common/AString.h>
#include <AUI/Common/AMap.h>


template<typename enum_t>
class AEnumerate {
public:
    static_assert(std::is_enum_v<enum_t>, "AEnumerate accepts only enums");



    template<enum_t... values>
    struct Values {

    };

    template<enum_t value>
    static AString valueName() {
#ifdef _MSC_VER
        AString s = __FUNCSIG__;
        auto end = s.rfind('>');
        size_t begin;
        begin = s.rfind('<') + 1;
        auto namespacePos = s.find("::", begin);
        if (namespacePos < end) {
            begin = namespacePos + 2;
        }
        AString result = {s.begin() + begin, s.begin() + end};
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
            result = result.mid(p + 2);
        }

#endif
        return result;
    }

    template<enum_t... values>
    static const AMap<AString, enum_t>& names() {
        static AMap<AString, enum_t> map = {
            {valueName<values>(), values}...
        };
        return map;
    }

    static const AMap<AString, enum_t>& all();

    static enum_t byName(const AString& name) {
        if (auto c = all().contains(name.uppercase())) {
            return c->second;
        }
        return (enum_t)0;
    }

private:
    template<enum_t... values>
    static const AMap<AString, enum_t>& allImpl(const Values<values...>& v) {
        return names<values...>();
    }
};

template<typename enum_t>
struct AEnumerateAllValues {
};

#define ENUM_VALUES(enum_t, ...) template<>\
struct AEnumerateAllValues<enum_t>{        \
    static inline constexpr AEnumerate<enum_t>::Values<__VA_ARGS__> get() {return {}; } \
};


template<typename enum_t>
const AMap<AString, enum_t>& AEnumerate<enum_t>::all() {
    auto v = AEnumerateAllValues<enum_t>::get();

    return allImpl(v);
}
