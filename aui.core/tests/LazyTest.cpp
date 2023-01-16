// AUI Framework - Declarative UI toolkit for modern C++17
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
// Created by alex2 on 31.08.2020.
//

#include <gtest/gtest.h>
#include <AUI/Traits/values.h>

TEST(Lazy, Unused) {
    aui::lazy<int> unused = [] {
        EXPECT_TRUE(false) << "initializer function called";
        return 0;
    };
}
TEST(Lazy, Used) {
    bool called = false;
    aui::lazy<int> used = [&] {
        EXPECT_FALSE(called) << "initializer function called twice";
        called = true;
        return 123;
    };
    ASSERT_FALSE(called);
    ASSERT_EQ(int(used), 123);
    ASSERT_TRUE(called);
    ASSERT_EQ(*used, 123);
    ASSERT_EQ(used, 123);

    ASSERT_EQ(int((const aui::lazy<int>)used), 123);
    ASSERT_EQ(*((const aui::lazy<int>)used), 123);
    ASSERT_EQ(((const aui::lazy<int>)used), 123);

    ASSERT_TRUE(called);
}
