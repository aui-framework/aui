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
// Created by alex2 on 31.08.2020.
//

#include <gtest/gtest.h>
#include <AUI/Common/AVector.h>
#include <AUI/Traits/iterators.h>
#include <array>

TEST(Iterators, Zip) {
    std::array<int, 3> ints = { 1, 2, 3 };
    std::array<std::string, 3> strings = { "one", "two", "three" };
    std::size_t index = 0;


    for (auto[i, s] : aui::zip(ints, strings)) {
        EXPECT_EQ(i, ints[index]);
        EXPECT_EQ(s, strings[index]);
        ++index;
    }
}


TEST(Iterators, ZipModification) {
    std::array<int, 3> vec1 = { 1, 2, 3 };
    std::array<int, 3> vec2 = { 4, 5, 6 };


    for (auto[v1, v2] : aui::zip(vec1, vec2)) {
        v1 += v2;
    }
    EXPECT_EQ(vec1, (std::array<int, 3>{5, 7, 9}));
}

TEST(Iterators, Reverse) {
    static_assert(aui::impl::is_forward_iterator<AVector<int>::iterator>, "AVector<int>::iterator should be a forward iterator");
    static_assert(!aui::impl::is_forward_iterator<AVector<int>::reverse_iterator>, "AVector<int>::reverse_iterator should not be a forward iterator");

    AVector<int> ints = { 1, 2, 3, 4 };

    EXPECT_EQ(*ints.begin(), 1);
    EXPECT_EQ(*aui::reverse_iterator_direction(ints.begin()), 1);

    EXPECT_EQ(*ints.rbegin(), 4);
    EXPECT_EQ(*aui::reverse_iterator_direction(ints.rbegin()), 4);

    AVector<int>::iterator iterator = aui::reverse_iterator_direction(ints.rbegin());
    EXPECT_EQ(*iterator, 4);
}

