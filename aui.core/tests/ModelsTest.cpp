// AUI Framework - Declarative UI toolkit for modern C++17
// Copyright (C) 2020-2022 Alex2772
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
#include <AUI/Model/AModels.h>
#include <AUI/Model/AListModel.h>


_<AListModel<int>> testModel() {
    return AListModel<int>::make({1, 5, 72, 23, 14, 35, 66, 37, 28, 19});
}

TEST(Models, RangesIncluding) {
    auto model = testModel();

    auto check = [](const AVector<AModelRange<int>>& v1, const AVector<AModelRange<int>>& v2) {
        ASSERT_EQ(v1, v2);
    };

    check(model->rangesIncluding([&](size_t i) {
        return i < 5;
    }), { model->range(0, 5) });

    check(model->rangesIncluding([&](size_t i) {
        return i < 5 || i >= 8;
    }), { model->range(0, 5), model->range(8, 10) });
    check(model->rangesIncluding([&](size_t i) {
        return i % 2 == 0;
    }), { model->range(0),
          model->range(2),
          model->range(4),
          model->range(6),
          model->range(8), });
}

/**
 * Converts int model to string model.
 */
TEST(Models, Adapter) {
    auto model = testModel();
    auto adaptedModel = AModels::adapt<AString>(model, [](int i) {
        return AString::number(i);
    });
    AVector<AString> expected = { "1", "5", "72", "23", "14", "35", "66", "37", "28", "19", };
    ASSERT_EQ(adaptedModel->toVector(), expected);
}

/**
 * Includes only even numbers.
 */
TEST(Models, Filter) {
    auto model = testModel();
    auto filteredModel = AModels::filter(model, [](int i) {
        return i % 2 == 0;
    });
    AVector<int> expected = { 72, 14, 66, 28 };
    ASSERT_EQ(filteredModel->toVector(), expected);
}

/**
 * Includes only even numbers, removes one even number and does lazyInvalidate.
 */
TEST(Models, FilterLazyInvalidate) {
    auto model = testModel();
    auto filteredModel = AModels::filter(model, [](int i) {
        return i % 2 == 0;
    });
    model->setItem(2, 71); // replace 72 with 71;
    filteredModel->lazyInvalidate();
    AVector<int> expected = { 14, 66, 28 };
    ASSERT_EQ(filteredModel->toVector(), expected);
}

/**
 * Includes only even numbers, adds one even number and does invalidate.
 */
TEST(Models, FilterInvalidate) {
    auto model = testModel();
    auto filteredModel = AModels::filter(model, [](int i) {
        return i % 2 == 0;
    });
    model->setItem(0, 2); // replace 1 with 2;
    model->setItem(1, 6); // replace 5 with 7;

    filteredModel->invalidate();

    AVector<int> expected = { 2, 6, 72, 14, 66, 28 };
    ASSERT_EQ(filteredModel->toVector(), expected);
}
