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

//
// Created by alex2 on 31.08.2020.
//

#include <range/v3/all.hpp>
#include <gtest/gtest.h>
#include <AUI/Common/AVector.h>
#include <AUI/Traits/iterators.h>
#include <array>
#include "AUI/Traits/any_range_view.h"

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

TEST(Iterators, DynRange1) {
    aui::any_range_view<int> ints = ranges::views::ints | ranges::views::take(10);
    EXPECT_EQ(ints | ranges::to_vector, std::vector({0, 1, 2, 3, 4, 5, 6, 7, 8, 9}));
}

TEST(Iterators, DynRange2) {
    aui::any_range_view<int> ints = ranges::views::ints | ranges::views::take(10) | ranges::views::filter([](int i) { return i % 2 == 0; });
    EXPECT_EQ(ints | ranges::to_vector, std::vector({0, 2, 4, 6, 8 }));
}

TEST(Iterators, DynRange3) {
    aui::any_range_view<int> ints = ranges::views::ints | ranges::views::take(10) | ranges::views::filter([](int i) { return i % 2 == 0; }) | ranges::to_vector;
    EXPECT_EQ(ints | ranges::to_vector, std::vector({0, 2, 4, 6, 8 }));
}

TEST(Iterators, DynRange4) {
    /// [DynRange4]
    AVector<int> elements{1,2,3};
    aui::any_range_view<int> ints = elements;
    EXPECT_EQ(ints | ranges::to_vector, std::vector({1, 2, 3 }));
    elements << 4;
    EXPECT_EQ(ints | ranges::to_vector, std::vector({1, 2, 3, 4 }));
    /// [DynRange4]
}

TEST(Iterators, DynRange5) {
    /// [DynRange5]
    AVector<int> elements{1,2,3};
    aui::any_range_view<int> ints = std::move(elements);
    EXPECT_EQ(ints | ranges::to_vector, std::vector({1, 2, 3 }));
    elements << 4;
    EXPECT_EQ(ints | ranges::to_vector, std::vector({1, 2, 3 }));
    /// [DynRange5]
}

TEST(Iterators, DynRangeNoCopy1) {
    AVector<std::unique_ptr<int>> elements{};
    aui::any_range_view<std::unique_ptr<int>> ints = std::move(elements);
}

TEST(Iterators, DynRangeNoCopy2) {
    AVector<std::unique_ptr<int>> elements{};
    aui::any_range_view<std::unique_ptr<int>> ints = elements;
}

TEST(Iterators, DynRangeCaps1) {
    aui::any_range_view<int> r = ranges::views::ints;
    EXPECT_TRUE(r.capabilities().implementsOperatorMinusMinus);
    auto it = r.begin();
    EXPECT_EQ(*it, 0);

    ++it;
    EXPECT_EQ(*it, 1);

    --it;
    EXPECT_EQ(*it, 0);
}

TEST(Iterators, DynRangeCaps2) {
    aui::any_range_view<int> r = ranges::views::ints | ranges::views::chunk_by([](int l, int r) { return l / 10 == r / 10; }) | ranges::views::transform([](const auto& i){ return 0; });
    EXPECT_FALSE(r.capabilities().implementsOperatorMinusMinus);
    auto it = r.begin();
    EXPECT_EQ(*it, 0);

    ++it;
    EXPECT_ANY_THROW(--it);
}
