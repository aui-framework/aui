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
#include <AUI/View/ATabView.h>

namespace {

class LayoutSpyView : public AView {
public:
    glm::ivec2 onIntrinsicMeasure(AConstraints constraints) override {
        return {
            constraints.isInlineTight() ? constraints.maxInline : 100,
            constraints.isBlockTight() ? constraints.maxBlock : 1000,
        };
    }

    void onLayout(int w, int h) override {
        AView::onLayout(w, h);
        lastLaidOutSize = { w, h };
    }

    glm::ivec2 lastLaidOutSize = {};
};

}

TEST(ATabView, PageContentGetsRemainingHeight) {
    auto content = _new<LayoutSpyView>();

    ATabView tabView;
    tabView.addTab(content, "Tab");
    tabView.layout(0, 0, 120, 100);

    const auto& row = tabView.getViews()[0];
    const auto& pageView = tabView.getViews()[1];

    EXPECT_EQ(pageView->getPosition().y, row->getHeight());
    EXPECT_EQ(pageView->getHeight(), 100 - row->getHeight());
    EXPECT_EQ(content->lastLaidOutSize.y, pageView->getHeight());
}
