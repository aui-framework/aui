//  AUI Framework - Declarative UI toolkit for modern C++20
//  Copyright (C) 2020-2023 Alex2772
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library. If not, see <http://www.gnu.org/licenses/>.

#include <gtest/gtest.h>

#include <AUI/Common/ASmallVector.h>

class SmallVector: public ::testing::Test {
protected:
    ASmallVector<std::unique_ptr<int>, 4> mVector;
};


TEST_F(SmallVector, Hack) {
    struct VectorHack {
        int* v0;
        int* v1;
        int* v2;
        int* v3;
        int* v4;
        int* v5;
        std::size_t size;
    };
    std::vector<int> data = { 1, 2, 3, 4, 5 };

    auto p = reinterpret_cast<VectorHack*>(&data);
    auto v = std::distance(p->v0, p->v1);
    EXPECT_EQ(p->size, 5);

    data.pop_back();
    EXPECT_EQ(p->size, 4);

    data.pop_back();
    EXPECT_EQ(p->size, 3);

    data.pop_back();
    EXPECT_EQ(p->size, 2);

    data.pop_back();
    EXPECT_EQ(p->size, 1);

    data.pop_back();
    EXPECT_EQ(p->size, 0);
}

TEST_F(SmallVector, PushBack) {
}