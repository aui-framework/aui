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

#if AUI_COROUTINES

#include "AUI/Util/AYieldGenerator.h"
#include <gtest/gtest.h>
#include <vector>
#include <string>

static constexpr auto LOG_TAG = "YieldGeneratorTest";

/**
 * @brief Test basic yield of integers.
 */
TEST(YieldGenerator, BasicIntegerYield) {
    auto generateNumbers = []() -> AYieldGenerator<int> {
        co_yield 1;
        co_yield 2;
        co_yield 3;
    };

    std::vector<int> values;
    for (auto value : generateNumbers()) {
        values.push_back(value);
    }

    EXPECT_EQ(values.size(), 3);
    EXPECT_EQ(values[0], 1);
    EXPECT_EQ(values[1], 2);
    EXPECT_EQ(values[2], 3);
}

/**
 * @brief Test yield with range-based for loop.
 */
TEST(YieldGenerator, RangeBasedForLoop) {
    auto generateSequence = []() -> AYieldGenerator<int> {
        for (int i = 0; i < 5; ++i) {
            co_yield i;
        }
    };

    int count = 0;
    int sum = 0;
    for (auto value : generateSequence()) {
        count++;
        sum += value;
    }

    EXPECT_EQ(count, 5);
    EXPECT_EQ(sum, 0 + 1 + 2 + 3 + 4);
}

/**
 * @brief Test empty generator.
 */
TEST(YieldGenerator, EmptyGenerator) {
    auto generateNothing = []() -> AYieldGenerator<int> {
        co_return;
    };

    std::vector<int> values;
    for (auto value : generateNothing()) {
        values.push_back(value);
    }

    EXPECT_TRUE(values.empty());
}

/**
 * @brief Test single yield.
 */
TEST(YieldGenerator, SingleYield) {
    auto generateOne = []() -> AYieldGenerator<int> {
        co_yield 42;
    };

    std::vector<int> values;
    for (auto value : generateOne()) {
        values.push_back(value);
    }

    EXPECT_EQ(values.size(), 1);
    EXPECT_EQ(values[0], 42);
}

/**
 * @brief Test yield of strings.
 */
TEST(YieldGenerator, StringYield) {
    auto generateStrings = []() -> AYieldGenerator<std::string> {
        co_yield "hello";
        co_yield "world";
        co_yield "test";
    };

    std::vector<std::string> values;
    for (auto value : generateStrings()) {
        values.push_back(value);
    }

    EXPECT_EQ(values.size(), 3);
    EXPECT_EQ(values[0], "hello");
    EXPECT_EQ(values[1], "world");
    EXPECT_EQ(values[2], "test");
}

/**
 * @brief Test move semantics of generator.
 */
TEST(YieldGenerator, MoveSemantics) {
    auto generateNumbers = []() -> AYieldGenerator<int> {
        co_yield 1;
        co_yield 2;
        co_yield 3;
    };

    auto gen1 = generateNumbers();
    auto gen2 = std::move(gen1);

    std::vector<int> values;
    for (auto value : gen2) {
        values.push_back(value);
    }

    EXPECT_EQ(values.size(), 3);
    EXPECT_EQ(values[0], 1);
    EXPECT_EQ(values[1], 2);
    EXPECT_EQ(values[2], 3);
}

/**
 * @brief Test move assignment of generator.
 */
TEST(YieldGenerator, MoveAssignment) {
    auto generateNumbers = []() -> AYieldGenerator<int> {
        co_yield 10;
        co_yield 20;
        co_yield 30;
    };

    AYieldGenerator<int> gen1 = generateNumbers();
    AYieldGenerator<int> gen2 = generateNumbers();
    
    gen1 = std::move(gen2);

    std::vector<int> values;
    for (auto value : gen1) {
        values.push_back(value);
    }

    EXPECT_EQ(values.size(), 3);
    EXPECT_EQ(values[0], 10);
    EXPECT_EQ(values[1], 20);
    EXPECT_EQ(values[2], 30);
}

/**
 * @brief Test generator with complex types (vector).
 */
TEST(YieldGenerator, ComplexTypeYield) {
    auto generateVectors = []() -> AYieldGenerator<std::vector<int>> {
        co_yield std::vector<int>{1, 2, 3};
        co_yield std::vector<int>{4, 5};
        co_yield std::vector<int>{6};
    };

    std::vector<std::vector<int>> values;
    for (auto value : generateVectors()) {
        values.push_back(value);
    }

    EXPECT_EQ(values.size(), 3);
    EXPECT_EQ(values[0].size(), 3);
    EXPECT_EQ(values[1].size(), 2);
    EXPECT_EQ(values[2].size(), 1);
}

/**
 * @brief Test iterator and sentinel comparison.
 */
TEST(YieldGenerator, IteratorSentinelComparison) {
    auto generateNumbers = []() -> AYieldGenerator<int> {
        co_yield 1;
        co_yield 2;
    };

    auto gen = generateNumbers();
    auto it = gen.begin();
    auto sentinel = gen.end();

    // Deference and move forward
    EXPECT_EQ(*it, 1);
    ++it;
    EXPECT_EQ(*it, 2);
    ++it;
    
    // Should be at end
    EXPECT_EQ(it, sentinel);
}

/**
 * @brief Test pointer operator of iterator.
 */
TEST(YieldGenerator, IteratorPointerOperator) {
    auto generateNumbers = []() -> AYieldGenerator<int> {
        co_yield 42;
    };

    auto gen = generateNumbers();
    auto it = gen.begin();

    EXPECT_EQ(*it.operator->(), 42);
}

/**
 * @brief Test that generator produces correct sequence.
 */
TEST(YieldGenerator, GeneratorProducesSequence) {
    auto generateNumbers = []() -> AYieldGenerator<int> {
        co_yield 1;
        co_yield 2;
    };

    auto gen = generateNumbers();
    
    // Collect all values from iteration
    std::vector<int> values;
    for (auto value : gen) {
        values.push_back(value);
    }

    EXPECT_EQ(values.size(), 2);
    EXPECT_EQ(values[0], 1);
    EXPECT_EQ(values[1], 2);
}

/**
 * @brief Test yield with conditional logic.
 */
TEST(YieldGenerator, ConditionalYield) {
    auto generateEvenNumbers = []() -> AYieldGenerator<int> {
        for (int i = 0; i < 10; ++i) {
            if (i % 2 == 0) {
                co_yield i;
            }
        }
    };

    std::vector<int> values;
    for (auto value : generateEvenNumbers()) {
        values.push_back(value);
    }

    EXPECT_EQ(values.size(), 5);
    EXPECT_EQ(values[0], 0);
    EXPECT_EQ(values[1], 2);
    EXPECT_EQ(values[2], 4);
    EXPECT_EQ(values[3], 6);
    EXPECT_EQ(values[4], 8);
}

/**
 * @brief Test yield with nested loops.
 */
TEST(YieldGenerator, NestedLoopsYield) {
    auto generatePairs = []() -> AYieldGenerator<int> {
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 2; ++j) {
                co_yield i * 10 + j;
            }
        }
    };

    std::vector<int> values;
    for (auto value : generatePairs()) {
        values.push_back(value);
    }

    EXPECT_EQ(values.size(), 6);
    EXPECT_EQ(values[0], 0);
    EXPECT_EQ(values[1], 1);
    EXPECT_EQ(values[2], 10);
    EXPECT_EQ(values[3], 11);
    EXPECT_EQ(values[4], 20);
    EXPECT_EQ(values[5], 21);
}

/**
 * @brief Test yield with references (lvalue references).
 */
TEST(YieldGenerator, ReferenceYield) {
    auto generateInts = []() -> AYieldGenerator<int&> {
        static int a = 10;
        static int b = 20;
        static int c = 30;
        co_yield a;
        co_yield b;
        co_yield c;
    };

    std::vector<int> values;
    for (auto value : generateInts()) {
        values.push_back(value);
    }

    EXPECT_EQ(values.size(), 3);
    EXPECT_EQ(values[0], 10);
    EXPECT_EQ(values[1], 20);
    EXPECT_EQ(values[2], 30);
}

/**
 * @brief Test sentinel equality with non-empty generator.
 */
TEST(YieldGenerator, SentinelEquality) {
    auto generateNumbers = []() -> AYieldGenerator<int> {
        co_yield 1;
        co_yield 2;
        co_yield 3;
    };

    auto gen = generateNumbers();
    auto it = gen.begin();
    auto sentinel = gen.end();

    EXPECT_NE(it, sentinel);

    for (int i = 0; i < 3; ++i) {
        ++it;
    }

    EXPECT_EQ(it, sentinel);
}

/**
 * @brief Test post-increment operator of iterator.
 */
TEST(YieldGenerator, PostIncrementOperator) {
    auto generateNumbers = []() -> AYieldGenerator<int> {
        co_yield 1;
        co_yield 2;
        co_yield 3;
    };

    auto gen = generateNumbers();
    auto it = gen.begin();

    // Post-increment returns void in this implementation
    EXPECT_EQ(*it, 1);
    it++;
    EXPECT_EQ(*it, 2);
}

/**
 * @brief Test large sequence generation.
 */
TEST(YieldGenerator, LargeSequence) {
    auto generateLargeSequence = []() -> AYieldGenerator<int> {
        for (int i = 0; i < 1000; ++i) {
            co_yield i;
        }
    };

    int count = 0;
    int sum = 0;
    for (auto value : generateLargeSequence()) {
        count++;
        sum += value;
    }

    EXPECT_EQ(count, 1000);
    // Sum of 0 to 999 is 999 * 1000 / 2
    EXPECT_EQ(sum, 999 * 500);
}

/**
 * @brief Test generator with state accumulation.
 */
TEST(YieldGenerator, StateAccumulation) {
    auto generateFibonacci = []() -> AYieldGenerator<int> {
        int a = 0, b = 1;
        for (int i = 0; i < 10; ++i) {
            co_yield a;
            int temp = a + b;
            a = b;
            b = temp;
        }
    };

    std::vector<int> values;
    for (auto value : generateFibonacci()) {
        values.push_back(value);
    }

    EXPECT_EQ(values.size(), 10);
    EXPECT_EQ(values[0], 0);
    EXPECT_EQ(values[1], 1);
    EXPECT_EQ(values[2], 1);
    EXPECT_EQ(values[3], 2);
    EXPECT_EQ(values[4], 3);
    EXPECT_EQ(values[5], 5);
    EXPECT_EQ(values[6], 8);
}

#endif
