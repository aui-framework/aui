/**
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
 * Original code located at https://github.com/Alex2772/aui
 * =====================================================================================================================
 */

//
// Created by alex2 on 31.08.2020.
//

#include <boost/test/unit_test.hpp>
#include <AUI/Common/AString.h>
#include <AUI/Traits/memory.h>

using namespace boost::unit_test;


enum MyEnum {
    INT,
    STRING,

    UNKNOWN,
};

template<typename EnumType, EnumType value>
struct RustEnumValue {

};

template <>
struct RustEnumValue<MyEnum, MyEnum::INT> {
    int value;
};

template <>
struct RustEnumValue<MyEnum, MyEnum::STRING> {
    AString value;
};

template<typename EnumType>
struct RustEnum {
private:
    template<int e>
    static constexpr size_t max(std::integer_sequence<int, e> sequence) {
        return sizeof(RustEnumValue<MyEnum, static_cast<EnumType>(e)>);
    }

    template<int a1, int a2, int ... args>
    static constexpr size_t max(std::integer_sequence<int, a1, a2, args...> sequence1) {
        size_t s1 = max(std::integer_sequence<int, a1>());
        size_t s2 = max(std::integer_sequence<int, a2, args...>());
        if (s1 > s2)
            return s1;
        return s2;
    }

    EnumType enumValue;
    char argBuffer[max(std::make_integer_sequence<int, (int)EnumType::UNKNOWN>())];

    void(*destructor)(char* buffer);

public:
    template<EnumType e>
    RustEnum(const RustEnumValue<EnumType, e>& value) {
        enumValue = e;
        aui::zero(argBuffer);
        *reinterpret_cast<RustEnumValue<EnumType, e>*>(argBuffer) = value;
        destructor = [](char* buffer) {
            reinterpret_cast<RustEnumValue<EnumType, e>*>(buffer)->~RustEnumValue<EnumType, e>();
        };
    }
    ~RustEnum() {
        destructor(argBuffer);
    }

    operator int() const {
        return enumValue;
    }

    template<EnumType e>
    const RustEnumValue<EnumType, e>& arg() const {
        assert(e == enumValue);
        return *reinterpret_cast<const RustEnumValue<EnumType, e>*>(argBuffer);
    }
};


BOOST_AUTO_TEST_SUITE(Enums)

    BOOST_AUTO_TEST_CASE(RustFlags) {
        RustEnum<MyEnum> d = RustEnumValue<MyEnum, STRING>{"govno v zhope"};

        switch (d) {
            case MyEnum::INT:
                BOOST_TEST(false);
                break;
            case MyEnum::STRING:
                auto s = d.arg<MyEnum::STRING>().value;
                BOOST_CHECK_EQUAL(s, "govno v zhope");
                break;
        }
    }

BOOST_AUTO_TEST_SUITE_END()