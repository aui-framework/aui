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
#include <AUI/View/AForEachUI.h>
#include <AUI/View/AScrollArea.h>

namespace {

class FixedRowView : public AView {
public:
    explicit FixedRowView(glm::ivec2 size) : mSize(size) {}

    glm::ivec2 onIntrinsicMeasure(AConstraints) override {
        return mSize;
    }

    AMinMaxAxis onComputeIntrinsicMinMaxAxis(int) override {
        return {
            .min = mSize.x,
            .max = mSize.x,
        };
    }

private:
    glm::ivec2 mSize;
};

}   // namespace

TEST(AForEachUI, MaterializesWholeFiniteModelWithoutViewport) {
    const AVector<int> items = { 1, 2, 3, 4, 5 };
    std::size_t created = 0;

    auto forEach = AUI_DECLARATIVE_FOR_EX(i, items, AVerticalLayout, &) {
        ++created;
        return _new<FixedRowView>(glm::ivec2(150, 20));
    };

    forEach->layout({ 0, 0 }, { 150, 200 });

    EXPECT_EQ(created, items.size());
    EXPECT_EQ(forEach->getViews().size(), items.size());
}

TEST(AForEachUI, ScrollAreaMaterializesOnlyInitialLazyWindow) {
    AVector<int> items;
    items.reserve(1000);
    for (int i = 0; i < 1000; ++i) {
        items << i;
    }

    std::size_t created = 0;

    AScrollArea scrollArea;
    scrollArea.setContents(AUI_DECLARATIVE_FOR_EX(i, items, AVerticalLayout, &) {
        ++created;
        return _new<FixedRowView>(glm::ivec2(150, 20));
    });

    scrollArea.layout({ 0, 0 }, { 150, 200 });

    EXPECT_GT(created, 0u);
    EXPECT_LT(created, 100u);
    EXPECT_TRUE(bool(scrollArea.verticalScrollbar()->getVisibility() & Visibility::FLAG_RENDER_NEEDED));
}
