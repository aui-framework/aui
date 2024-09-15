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

#include <gtest/gtest.h>

#include <AUI/Common/ASmallVector.h>

class StaticVector: public ::testing::Test {
};


TEST_F(StaticVector, PushBack) {
    GTEST_FLAG_SET(death_test_style, "threadsafe");
    AStaticVector<std::unique_ptr<int>, 4> vector;

    for (int i = 0; i < 4; ++i) {
        vector.push_back(std::make_unique<int>(i));
    }
    EXPECT_DEATH({ vector.push_back(std::make_unique<int>(0));}, "");

    EXPECT_EQ(*vector[0], 0);
    EXPECT_EQ(*vector[1], 1);
    EXPECT_EQ(*vector[2], 2);
    EXPECT_EQ(*vector[3], 3);


    vector.pop_back();
    EXPECT_DEATH({ vector[3];}, "");

    vector.pop_back();
    vector.pop_back();
    vector.pop_back();

    EXPECT_DEATH({ vector.pop_back(); }, "");
}

TEST_F(StaticVector, Deallocation) {
    _weak<int> v0, v1, v2, v3;
    {
        AStaticVector<_<int>, 4> vector;
        for (int i = 0; i < 4; ++i) {
            vector.push_back(std::make_shared<int>(i));
        }

        v0 = vector[0].weak();
        v1 = vector[1].weak();
        v2 = vector[2].weak();
        v3 = vector[3].weak();

        vector.pop_back();
        EXPECT_EQ(vector.size(), 3);
        EXPECT_FALSE(v0.expired());
        EXPECT_FALSE(v1.expired());
        EXPECT_FALSE(v2.expired());
        EXPECT_TRUE(v3.expired());
    }
    EXPECT_TRUE(v0.expired());
    EXPECT_TRUE(v1.expired());
    EXPECT_TRUE(v2.expired());
    EXPECT_TRUE(v3.expired());
}


TEST_F(StaticVector, PopFront) {
    _weak<int> v0, v1, v2, v3;
    {
        AStaticVector<_<int>, 4> vector;
        for (int i = 0; i < 4; ++i) {
            vector.push_back(std::make_shared<int>(i));
        }

        v0 = vector[0].weak();
        v1 = vector[1].weak();
        v2 = vector[2].weak();
        v3 = vector[3].weak();

        vector.pop_front();
        EXPECT_EQ(vector.size(), 3);
        EXPECT_TRUE(v0.expired());
        EXPECT_FALSE(v1.expired());
        EXPECT_FALSE(v2.expired());
        EXPECT_FALSE(v3.expired());

        EXPECT_EQ(*vector[0], 1);
        EXPECT_EQ(*vector[1], 2);
        EXPECT_EQ(*vector[2], 3);
    }
    EXPECT_TRUE(v0.expired());
    EXPECT_TRUE(v1.expired());
    EXPECT_TRUE(v2.expired());
    EXPECT_TRUE(v3.expired());
}


TEST_F(StaticVector, InsertMiddle) {
    AStaticVector<std::unique_ptr<int>, 4> vector;

    for (int i = 0; i < 3; ++i) {
        vector.push_back(std::make_unique<int>(i));
    }

    vector.insert(vector.begin() + 1, std::make_unique<int>(228));

    EXPECT_EQ(*vector[0], 0);
    EXPECT_EQ(*vector[1], 228);
    EXPECT_EQ(*vector[2], 1);
    EXPECT_EQ(*vector[3], 2);
}

TEST_F(StaticVector, InsertEnd) {
    AStaticVector<std::unique_ptr<int>, 4> vector;

    for (int i = 0; i < 3; ++i) {
        vector.push_back(std::make_unique<int>(i));
    }

    vector.insert(vector.end(), std::make_unique<int>(228));

    EXPECT_EQ(*vector[0], 0);
    EXPECT_EQ(*vector[1], 1);
    EXPECT_EQ(*vector[2], 2);
    EXPECT_EQ(*vector[3], 228);
}


TEST_F(StaticVector, PushFront) {
    AStaticVector<std::unique_ptr<int>, 4> vector;

    for (int i = 0; i < 3; ++i) {
        vector.push_back(std::make_unique<int>(i));
    }

    vector.push_front(std::make_unique<int>(228));

    EXPECT_EQ(*vector[0], 228);
    EXPECT_EQ(*vector[1], 0);
    EXPECT_EQ(*vector[2], 1);
    EXPECT_EQ(*vector[3], 2);
}

TEST_F(StaticVector, EraseFromMiddle) {
    AStaticVector<std::unique_ptr<int>, 4> vector;

    for (int i = 0; i < 4; ++i) {
        vector.push_back(std::make_unique<int>(i));
    }

    EXPECT_EQ(*vector[0], 0);
    EXPECT_EQ(*vector[1], 1);
    EXPECT_EQ(*vector[2], 2);
    EXPECT_EQ(*vector[3], 3);

    vector.erase(vector.begin() + 1);

    EXPECT_EQ(vector.size(), 3);
    EXPECT_EQ(*vector[0], 0);
    EXPECT_EQ(*vector[1], 2);
    EXPECT_EQ(*vector[2], 3);
}
