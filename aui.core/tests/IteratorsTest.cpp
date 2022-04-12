/**
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
 */

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
    AVector<int> ints = { 1, 2, 3, 4 };

    EXPECT_EQ(*ints.begin(), 1);
    EXPECT_EQ(*aui::reverse_iterator_direction(ints.begin()), 1);

    EXPECT_EQ(*ints.rbegin(), 4);
    EXPECT_EQ(*aui::reverse_iterator_direction(ints.rbegin()), 4);
}

