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
#include <AUI/Model/ATreeModel.h>
#include <AUI/View/ATreeView.h>

namespace {

class FixedWidthTreeItemView : public AView {
public:
    FixedWidthTreeItemView(int width, int height) : mWidth(width), mHeight(height) {}

    AMinMaxAxis onComputeIntrinsicMinMaxAxis(int) override {
        return {
            .min = mWidth,
            .max = mWidth,
        };
    }

private:
    int mWidth;
    int mHeight;
};

const _<AScrollbar>& verticalScrollbar(ATreeView& treeView) {
    return _cast<AScrollbar>(treeView.getViews().at(1));
}

const _<AScrollbar>& horizontalScrollbar(ATreeView& treeView) {
    return _cast<AScrollbar>(treeView.getViews().at(2));
}

}   // namespace

TEST(ATreeView, ShowsHorizontalScrollbarForWideRows) {
    auto model = _new<ATreeModel<AString>>(AVector<ATreeModel<AString>::Item> {
        { .value = "row" },
    });

    ATreeView treeView;
    treeView.setViewFactory([](const _<ITreeModel<AString>>&, const ATreeModelIndex&) {
        return _new<FixedWidthTreeItemView>(150, 20);
    });
    treeView.setModel(model);
    treeView.setSize({ 100, 40 });

    EXPECT_TRUE(static_cast<bool>(horizontalScrollbar(treeView)->getVisibility() & Visibility::FLAG_RENDER_NEEDED));
    EXPECT_FALSE(static_cast<bool>(verticalScrollbar(treeView)->getVisibility() & Visibility::FLAG_RENDER_NEEDED));
    EXPECT_EQ(horizontalScrollbar(treeView)->getPosition().y, 25);
    EXPECT_EQ(horizontalScrollbar(treeView)->getWidth(), 100);
}

TEST(ATreeView, SelectScrollsHorizontallyToRevealWideRows) {
    auto model = _new<ATreeModel<AString>>(AVector<ATreeModel<AString>::Item> {
        { .value = "row" },
    });

    ATreeView treeView;
    treeView.setViewFactory([](const _<ITreeModel<AString>>&, const ATreeModelIndex&) {
        return _new<FixedWidthTreeItemView>(150, 20);
    });
    treeView.setModel(model);
    treeView.setSize({ 100, 40 });

    treeView.select(model->indexOfChild(0, 0, ATreeModelIndex::ROOT));

    EXPECT_GT(horizontalScrollbar(treeView)->getCurrentScroll(), 0);
}
