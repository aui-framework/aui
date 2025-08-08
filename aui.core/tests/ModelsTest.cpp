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

#include <gtest/gtest.h>
#include <AUI/Model/AModels.h>
#include <AUI/Model/AListModel.h>
#include <gmock/gmock.h>
#include "AUI/Util/kAUI.h"


namespace {
    _<AListModel<int>> testModel() {
        return AListModel<int>::make({1, 5, 72, 23, 14, 35, 66, 37, 28, 19});
    }

    class Receiver : public AObject {
    public:
        MOCK_METHOD(void, receive, (AListModelRange<int>));
    };
}

TEST(Models, Insert) {
    auto model = _new<AListModel<int>>();
    auto receiver = _new<Receiver>();
    AObject::connect(model->dataInserted, AUI_SLOT(receiver)::receive);

    testing::InSequence s;
    EXPECT_CALL(*receiver, receive(AListModelRange<int>(AListModelIndex(0), AListModelIndex(1), model)));
    EXPECT_CALL(*receiver, receive(AListModelRange<int>(AListModelIndex(1), AListModelIndex(2), model)));
    EXPECT_CALL(*receiver, receive(AListModelRange<int>(AListModelIndex(0), AListModelIndex(1), model)));

    model << 228;
    EXPECT_EQ(*std::prev(model->end()), 228);
    EXPECT_EQ(model->listSize(), 1);

    model << 322;
    EXPECT_EQ(*std::prev(model->end()), 322);
    EXPECT_EQ(model->listSize(), 2);

    model->insert(model->begin(), 0);
    EXPECT_EQ(*model->begin(), 0);
    EXPECT_EQ(model->listSize(), 3);
}

TEST(Models, RangesIncluding) {
    auto model = testModel();

    auto check = [](const AVector<AListModelRange<int>>& v1, const AVector<AListModelRange<int>>& v2) {
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
