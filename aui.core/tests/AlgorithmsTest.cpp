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
#include <AUI/Traits/algorithms.h>
#include <AUI/Common/AVector.h>

void performSearchOn(const AVector<int>::const_iterator& begin, const AVector<int>::const_iterator& end) {
    auto theoreticalComplexity = std::log2(end - begin) + 1; // for complexity check

    // perform search of the every value in the range
    for (auto it = begin; it != end; ++it) {
        auto searchingFor = *it;
        size_t calledTimes = 0; // for complexity check

        auto resultToCheck = aui::binary_search(begin, end, [&](const AVector<int>::const_iterator& it) {
            ++calledTimes;
            if (*it == searchingFor) {
                return aui::BinarySearchResult::MATCH;
            }
            if (*it > searchingFor) return aui::BinarySearchResult::LEFT;
            return aui::BinarySearchResult::RIGHT;
        });

        // perform result check
        ASSERT_EQ(*resultToCheck, searchingFor);
        if (*resultToCheck != searchingFor) {
            ADD_FAILURE() << "Check failed!";
        }

        // check we fit into the log2(size) complexity
        ASSERT_LE(calledTimes, theoreticalComplexity);
    }

    // also check if we search for *unexisting* value the algorithm results end arg
    ASSERT_TRUE((aui::binary_search(begin, end, [](const AVector<int>::const_iterator& it) { return aui::BinarySearchResult::LEFT;  }) == end)) << "binary_search found unexisting element (wtf?)";
    ASSERT_TRUE((aui::binary_search(begin, end, [](const AVector<int>::const_iterator& it) { return aui::BinarySearchResult::RIGHT; }) == end)) << "binary_search found unexisting element (wtf?)";
}

void performSearchOn(const AVector<int>& v) {
    performSearchOn(v.begin(),  v.end());
}

TEST(Algorithms, BinarySearch1) {
    AVector<int> dataset = { 1, 2, 7, 9, 11, 21, 33, 64, 96, 102, 299, 412414, 58989129, 58989131, 58989153 };

    // search all variants starting with empty range and ending with full vector
    for (auto it = dataset.begin(); ; ++it) {
        performSearchOn(dataset.begin(), it);

        // detect the end of the vector
        if (it == dataset.end()) {
            break;
        }
    }

    // try to confuse binary search algorithm
    dataset[0] = 228;
    ASSERT_TRUE(aui::binary_search(dataset.begin(), dataset.end(), [](const AVector<int>::const_iterator& it) { return aui::BinarySearchResult::LEFT; }) == dataset.end()) << "binary_search found unexisting element (wtf?)";
}

TEST(Algorithms, BinarySearchNearestToZero) {
    AVector<int> dataset = { 1, 4, 7, 9, 11, 21, 33, 64, 96, 102, 299, 412414, 58989129, 58989131, 58989153 };

    // search for all elements
    for (const auto& value : dataset) {
        auto result = aui::binary_search(dataset.begin(),
                                         dataset.end(),
                                         aui::BinarySearchNearestToZero([&](const AVector<int>::iterator& iterator) {
                                             return *iterator - value;
                                         }, aui::range(dataset)));
        ASSERT_FALSE(result == dataset.end());
        ASSERT_EQ(value, *result);
    }
}
TEST(Algorithms, BinarySearchNearestToZeroWithOffset) {
    AVector<int> dataset = { 1, 4, 7, 11, 21, 33, 64, 96, 102, 299, 412414, 58989129, 58989153 };

    // search for all elements
    for (const auto& value : dataset) {
        for (auto offset : { -1, 1 }) { // simulate value is unknown; a small offset to the predicate
            auto result = aui::binary_search(dataset.begin(),
                                             dataset.end(),
                                             aui::BinarySearchNearestToZero([&](const AVector<int>::iterator& iterator) {
                                                 return *iterator - (value + offset);
                                             }, aui::range(dataset)));
            ASSERT_FALSE(result == dataset.end());
            ASSERT_EQ(value, *result);
        }
    }
}
