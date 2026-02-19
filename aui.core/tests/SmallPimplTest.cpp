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

#include "AUI/Image/APixelFormat.h"

// Define a simple interface used by the small_pimpl tests.
// It contains a single virtual method `test()` that will be
// overridden by mock implementations.
struct MyInterface {
    int field = 0;
    virtual ~MyInterface() = default;
    virtual void test() = 0;
};

thread_local bool destructorCalled = false;

// implementation that is intentionally small enough to be
// stored on the stack by `small_pimpl`.
struct small {
    // The test expects this mock to be allocated on the stack.
    static constexpr bool EXPECT_HEAP_ALLOCATED = false;
    class Mock : public MyInterface {
    public:
        static constexpr bool EXPECT_HEAP_ALLOCATED = false;
        MOCK_METHOD(void, test, (), (override));

        ~Mock() override {
            destructorCalled = true;
        }
    };

    class CopyOnly : public MyInterface {
    public:
        CopyOnly() = default;
        CopyOnly(const CopyOnly&) = default;
        CopyOnly(CopyOnly&&) = delete;

        void test() override {}
        ~CopyOnly() override {
            destructorCalled = true;
        }
    };

    class MoveOnly : public MyInterface {
    public:
        MoveOnly() = default;
        MoveOnly(const MoveOnly&) = delete;
        MoveOnly(MoveOnly&&) = default;

        void test() override {}
        ~MoveOnly() override {
            destructorCalled = true;
        }
    };
};


// implementation that is intentionally large, forcing
// `small_pimpl` to allocate it on the heap.
struct big {
    // The test expects this mock to be allocated on the heap.
    static constexpr bool EXPECT_HEAP_ALLOCATED = true;
    struct Mock : public MyInterface {
        char padding[1024];
        MOCK_METHOD(void, test, (), (override));

        ~Mock() override {
            destructorCalled = true;
        }
    };
    class CopyOnly : public MyInterface {
    public:
        char padding[1024];
        CopyOnly() = default;
        CopyOnly(const CopyOnly&) = default;
        CopyOnly(CopyOnly&&) = delete;

        void test() override {}
        ~CopyOnly() override {
            destructorCalled = true;
        }
    };

    class MoveOnly : public MyInterface {
    public:
        char padding[1024];
        MoveOnly() = default;
        MoveOnly(const MoveOnly&) = delete;
        MoveOnly(MoveOnly&&) = default;

        void test() override {}
        ~MoveOnly() override {
            destructorCalled = true;
        }
    };
};

static constexpr std::size_t SMALL_SIZE = 128;
static_assert(sizeof(small::Mock) <= SMALL_SIZE);
static_assert(sizeof(small::CopyOnly) <= SMALL_SIZE);
static_assert(sizeof(small::MoveOnly) <= SMALL_SIZE);
static_assert(sizeof(big::Mock) > SMALL_SIZE);
static_assert(sizeof(big::CopyOnly) > SMALL_SIZE);
static_assert(sizeof(big::MoveOnly) > SMALL_SIZE);

using MyPimpl = aui::small_pimpl<MyInterface, SMALL_SIZE>;

// Test fixture template that will be instantiated with each mock type.
template <typename T>
class SmallPimpl : public testing::Test {};

using TestTypes = testing::Types<small, big>;
TYPED_TEST_SUITE(SmallPimpl, TestTypes);

// Verify that `small_pimpl` chooses the correct storage location
// (stack vs heap) based on the size of the concrete type.
TYPED_TEST(SmallPimpl, AllocationDecision) {
    using MockInterface = TypeParam::Mock; // either small::Mock or big::Mock

    MyPimpl pimpl{std::in_place_type<MockInterface>};
    EXPECT_EQ(pimpl.isOnHeap(), TypeParam::EXPECT_HEAP_ALLOCATED);
}

// Test that the mock's `test()` method is correctly invoked through
// the `small_pimpl` interface.
TYPED_TEST(SmallPimpl, MockTestInvocation) {
    using MockInterface = TypeParam::Mock; // either small::Mock or big::Mock

    MyPimpl pimpl{std::in_place_type<MockInterface>};
    {
        auto* mock = dynamic_cast<MockInterface*>(pimpl.ptr());
        ASSERT_NE(mock, nullptr);
        EXPECT_CALL(*mock, test()).Times(1);
    }
    pimpl->test();
}

// Ensure that the destructor of the concrete type is called when
// the `small_pimpl` goes out of scope.
TYPED_TEST(SmallPimpl, Destructor) {
    using MockInterface = TypeParam::Mock; // either small::Mock or big::Mock

    {
        MyPimpl pimpl{std::in_place_type<MockInterface>};
        destructorCalled = false;
    }
    EXPECT_TRUE(destructorCalled);
}

TYPED_TEST(SmallPimpl, Copy) {
    using CopyOnly = TypeParam::CopyOnly; // either small::CopyOnly or big::CopyOnly

    {
        MyPimpl pimpl1{std::in_place_type<CopyOnly>};
        pimpl1->field = 42;
        {
            MyPimpl pimpl2{pimpl1};
            EXPECT_EQ(pimpl2->field, 42);
            destructorCalled = false;
        }
        EXPECT_TRUE(destructorCalled);
        destructorCalled = false;
    }
    EXPECT_TRUE(destructorCalled);
}

TYPED_TEST(SmallPimpl, Move) {
    using MoveOnly = TypeParam::MoveOnly; // either small::MoveOnly or big::MoveOnly

    {
        MyPimpl pimpl1{std::in_place_type<MoveOnly>};
        pimpl1->field = 42;
        {
            EXPECT_ANY_THROW(MyPimpl{pimpl1});
            MyPimpl pimpl2{std::move(pimpl1)};
            EXPECT_EQ(pimpl2->field, 42);
            destructorCalled = false;
        }
        EXPECT_TRUE(destructorCalled);
        destructorCalled = false;
    }
    EXPECT_FALSE(destructorCalled);
}



