/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2025 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, you can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "AUI/Util/SmallPimpl.h"


struct MyInterface {
    int field = 0;
    virtual ~MyInterface() = default;
    virtual void test() = 0;
};

thread_local bool destructorCalled = false;

class MockInterfaceSmall : public MyInterface {
public:
    static constexpr bool EXPECT_HEAP_ALLOCATED = false;
    MOCK_METHOD(void, test, (), (override));

    ~MockInterfaceSmall() override {
        destructorCalled = true;
    }
};

static constexpr std::size_t SMALL_SIZE = 128;
static_assert(sizeof(MockInterfaceSmall) < SMALL_SIZE);

class MockInterfaceBig : public MyInterface {
public:
    static constexpr bool EXPECT_HEAP_ALLOCATED = true;
    char padding[1024];
    MOCK_METHOD(void, test, (), (override));

    ~MockInterfaceBig() override {
        destructorCalled = true;
    }
};

template <typename T>
class SmallPimpl : public testing::Test {};

using TestTypes = testing::Types<MockInterfaceSmall, MockInterfaceBig>;
TYPED_TEST_SUITE(SmallPimpl, TestTypes);

TYPED_TEST(SmallPimpl, AllocationDecision) {
    using MockInterface = TypeParam; // either MockInterfaceSmall or MockInterfaceBig

    aui::small_pimpl<MyInterface, SMALL_SIZE> pimpl{std::in_place_type<MockInterface>};
    EXPECT_EQ(pimpl.isOnHeap(), MockInterface::EXPECT_HEAP_ALLOCATED);
}

TYPED_TEST(SmallPimpl, MockTestInvocation) {
    using MockInterface = TypeParam; // either MockInterfaceSmall or MockInterfaceBig

    aui::small_pimpl<MyInterface, SMALL_SIZE> pimpl{std::in_place_type<MockInterface>};
    {
        auto* mock = dynamic_cast<MockInterface*>(pimpl.ptr());
        ASSERT_NE(mock, nullptr);
        EXPECT_CALL(*mock, test()).Times(1);
    }
    pimpl->test();
}

TYPED_TEST(SmallPimpl, Destructor) {
    using MockInterface = TypeParam; // either MockInterfaceSmall or MockInterfaceBig

    {
        aui::small_pimpl<MyInterface, SMALL_SIZE> pimpl{std::in_place_type<MockInterface>};
        destructorCalled = false;
    }
    EXPECT_TRUE(destructorCalled);
}


