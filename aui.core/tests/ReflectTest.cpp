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

//
// Created by alex2 on 30.08.2020.
//

#include <gtest/gtest.h>
#include <AUI/Common/AString.h>
#include <AUI/Common/AMap.h>
#include <AUI/Reflect/AClass.h>
#include <AUI/Reflect/AEnumerate.h>
#include <AUI/Reflect/AReflect.h>
#include <AUI/Model/AListModel.h>
#include "ReflectionTest.h"

enum ATest {
    VALUE1,
    VALUE2,
    VALUE3 = 1000,
    VALUE4 = 10000,
};

namespace namespaceeee {
    enum ATest {
        V1,
        V2,
        V3,
    };
    enum class ATest2 {
        TEST2_1,
        TEST2_2,
        TEST2_3,
    };
}

struct MyStruct {
};

namespace AzazaATest {
    struct ATest {
    };
}

enum class EnumWithoutEnumValue {
    SOME_VALUE1
};

template<typename T>
inline std::ostream &operator<<(std::ostream &o, ATest t) {
    o << int(t);
    return o;
}

template<typename T>
inline std::ostream &operator<<(std::ostream &o, namespaceeee::ATest t) {
    o << int(t);
    return o;
}

template<typename T>
inline std::ostream &operator<<(std::ostream &o, namespaceeee::ATest2 t) {
    o << int(t);
    return o;
}

template<typename T>
inline std::ostream &operator<<(std::ostream &o, EnumWithoutEnumValue t) {
    o << int(t);
    return o;
}


AUI_ENUM_VALUES(ATest, VALUE1, VALUE2, VALUE3, VALUE4)

AUI_ENUM_VALUES(namespaceeee::ATest, namespaceeee::V1, namespaceeee::V2, namespaceeee::V3)

AUI_ENUM_VALUES(namespaceeee::ATest2, namespaceeee::ATest2::TEST2_1, namespaceeee::ATest2::TEST2_2,
                namespaceeee::ATest2::TEST2_3)


TEST(Reflect, NameClass) {
    ASSERT_EQ(AClass<AString>::name(), "AString");
    ASSERT_EQ(AClass<AzazaATest::ATest>::nameWithoutNamespace(), "ATest");
}


TEST(Reflect, NameStruct) {
    ASSERT_EQ(AClass<MyStruct>::name(), "MyStruct");
}

TEST(Reflect, NameEnum) {

    ASSERT_EQ((AClass<ATest>::name()), "ATest");
}

TEST(Reflect, EnumerateAll) {
    AMap<AString, ATest> ref = {
            {"VALUE1", VALUE1},
            {"VALUE2", VALUE2},
            {"VALUE3", VALUE3},
            {"VALUE4", VALUE4},
    };

    auto test = AEnumerate<ATest>::nameToValueMap();
    ASSERT_EQ(test, ref);
}

TEST(Reflect, NamespaceEnumerateNames) {
    AMap<AString, namespaceeee::ATest> ref = {
            {"V1", namespaceeee::V1},
            {"V2", namespaceeee::V2},
            {"V3", namespaceeee::V3},
    };
    auto test = AEnumerate<namespaceeee::ATest>::nameToValueMap();
    ASSERT_EQ(test, ref);
}

TEST(Reflect, NamespaceEnumerateEnumClassNames) {
    AMap<AString, namespaceeee::ATest2> ref = {
            {"TEST2_1", namespaceeee::ATest2::TEST2_1},
            {"TEST2_2", namespaceeee::ATest2::TEST2_2},
            {"TEST2_3", namespaceeee::ATest2::TEST2_3},
    };
    auto test = AEnumerate<namespaceeee::ATest2>::nameToValueMap();
    ASSERT_TRUE((test == ref));
}

TEST(Reflect, EnumWithoutEnumValueCase) {
    ASSERT_EQ(AEnumerate<EnumWithoutEnumValue>::valueName<EnumWithoutEnumValue::SOME_VALUE1>(), "SOME_VALUE1");
}

#ifndef AUI_REFLECT_FIELD_NAMES_ENABLED
#   if  (__cplusplus >= 202002L) || (defined(_MSVC_LANG) && (_MSVC_LANG >= 202002L))
#       if (defined(__cpp_nontype_template_args) && __cpp_nontype_template_args >= 201911) \
 || (defined(__clang_major__) && __clang_major__ >= 12)
#           define  AUI_REFLECT_FIELD_NAMES_ENABLED 1
#       else
#           define  AUI_REFLECT_FIELD_NAMES_ENABLED 0
#       endif
#   else
#       define  AUI_REFLECT_FIELD_NAMES_ENABLED 0
#   endif
#endif

namespace aui::reflect {

    namespace detail {
#if AUI_COMPILER_MSVC && AUI_REFLECT_FIELD_NAMES_ENABLED

        /**
         * @brief External linkage wrapper, so T is not enforced to be externally linked.
         */
        template<class T>
        struct wrapper {
            const T value;
        };

        /**
         * @brief Link time assert. If linker fails to link with it, it means that fake_object used in run time.
         */
        template<class T>
        extern const wrapper<T> DO_NOT_USE_REFLECTION_WITH_LOCAL_TYPES;

        /**
         * @brief For returning non-default constructible types.
         * @tparam T
         * @details
         * Neither std::declval nor unsafe_declval are suitable here.
         */
        template<class T>
        constexpr const T& fake_object() noexcept {
            return DO_NOT_USE_REFLECTION_WITH_LOCAL_TYPES<T>.value;
        }


        template<class UniqueKey, // https://developercommunity.visualstudio.com/t/__FUNCSIG__-outputs-wrong-value-with-C/10458554
                 auto M>
        consteval std::string_view name_of_field_impl() noexcept {
            std::string_view s = __FUNCSIG__;
            s = s.substr(s.rfind("->value->"));
            s = s.substr(sizeof("->value->") - 1);
            s = s.substr(0, s.rfind(">(void)"));
            return s;
        }


        template<class UniqueKey, // https://developercommunity.visualstudio.com/t/__FUNCSIG__-outputs-wrong-value-with-C/10458554
                 auto M>
        consteval std::string_view name_of_field_impl_method() noexcept {
            std::string_view s = __FUNCSIG__;
            s = s.substr(s.rfind(':') + 1);
            s = s.substr(0, s.find('('));
            return s;
        }
#endif
    }

    template<auto M>
    struct member_v : member<decltype(M)> {
#if AUI_REFLECT_FIELD_NAMES_ENABLED
    private:
        static consteval std::string_view getName() {
#if AUI_COMPILER_MSVC
            if constexpr (requires { typename member_v::return_t; }) {
                return detail::name_of_field_impl_method<typename member_v::clazz, M>();
            } else {
                return detail::name_of_field_impl<typename member_v::clazz, std::addressof(std::addressof(detail::fake_object<typename member_v::clazz>())->*M)>();
            }
#elif AUI_COMPILER_CLANG
            std::string_view s = __PRETTY_FUNCTION__;
            {
                auto last = s.rfind(']');
                auto begin = s.rfind('&');
                s = s.substr(begin, last - begin);
            }
            if (auto c = s.rfind(':')) {
                s = s.substr(c + 1);
            }
            return s;
#else
            std::string_view s = __PRETTY_FUNCTION__;
            {
                auto last = s.rfind(';');
                auto begin = s.rfind('&');
                s = s.substr(begin, last - begin);
            }
            if (auto c = s.rfind(':')) {
                s = s.substr(c + 1);
            }
            return s;
#endif
        }

    public:
        static constexpr std::string_view name = getName();
#endif
    };
}

/// [member_v]
struct SomeStruct {
    int someInt;
    std::string someString;

    long someFunc(float arg) {}
};
/// [member_v]

TEST(Reflect, FieldName) {
    /// [member_v]


    EXPECT_EQ(aui::reflect::member_v<&SomeStruct::someInt>::name, "someInt");
    static_assert(std::is_same_v<aui::reflect::member_v<&SomeStruct::someInt>::type, int>);

    EXPECT_EQ(aui::reflect::member_v<&SomeStruct::someString>::name, "someString");
    static_assert(std::is_same_v<aui::reflect::member_v<&SomeStruct::someString>::type, std::string>);

    EXPECT_EQ(aui::reflect::member_v<&SomeStruct::someFunc>::name, "someFunc");
    static_assert(std::is_same_v<aui::reflect::member_v<&SomeStruct::someFunc>::return_t, long>);
    static_assert(std::is_same_v<aui::reflect::member_v<&SomeStruct::someFunc>::args, std::tuple<float>>);
    static_assert(!aui::reflect::member_v<&SomeStruct::someFunc>::is_const);
    static_assert(!aui::reflect::member_v<&SomeStruct::someFunc>::is_noexcept);
    /// [member_v]
}

TEST(Reflect, FieldCount) {
    struct Data {
        int a;
        std::string b;
    };
    EXPECT_EQ(aui::reflect::detail::fields_count<Data>(), 2);
}
