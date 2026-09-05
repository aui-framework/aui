/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2026 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <gtest/gtest.h>
#include <AUI/Model/AListModel.h>
#include <AUI/View/AListView.h>

TEST(AListView, MeasureWidthStaysStableWhenModelSizeChanges) {
    auto model = _new<AListModel<AString>>();
    (*model) << "First item";

    AListView listView(model);

    const auto oneItem = listView.measure(AConstraints::fixedInline(0));

    (*model) << "Second item";
    const auto twoItems = listView.measure(AConstraints::fixedInline(0));

    model->pop_back();
    const auto oneItemAgain = listView.measure(AConstraints::fixedInline(0));

    EXPECT_EQ(oneItem.x, 4);
    EXPECT_EQ(twoItems.x, oneItem.x);
    EXPECT_EQ(oneItemAgain.x, oneItem.x);
    EXPECT_GT(twoItems.y, oneItem.y);
    EXPECT_EQ(oneItemAgain.y, oneItem.y);
}
