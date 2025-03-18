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
// Created by alex2 on 31.08.2020.
//

#include <range/v3/all.hpp>
#include <gtest/gtest.h>
#include <AUI/Common/AVector.h>
#include <AUI/Traits/iterators.h>
#include <array>
#include "AUI/Traits/dyn_range.h"

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

TEST(Iterators, DynRange) {
    aui::dyn_range<int> ints = ranges::views::ints | ranges::views::take(10);


    EXPECT_EQ(ints.size(), 10);
}


