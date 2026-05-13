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
#include <AUI/View/ASplitter.h>
#include <AUI/View/ASplitterHelper.h>
#include <AUI/Util/SplitterSizeInjector.h>
#include <AUI/View/AView.h>
#include <functional>

template<typename T>
inline std::ostream& operator<<(std::ostream& o, const glm::tvec2<T>& v) {
    return o << "{ " << v.x << ", " << v.y << " }";
}

namespace {

class FakeLayoutItem : public AView {
public:
    std::function<int(int)> preferredWidth = [](int) { return 0; };
    std::function<int(int)> preferredHeight = [](int) { return 0; };

    glm::ivec2 onIntrinsicMeasure(AConstraints constraints) override {
        const int widthConstraint = constraints.isUnlimitedInline() ? -1 : constraints.maxInline;
        const int heightConstraint = constraints.isUnlimitedBlock() ? -1 : constraints.maxBlock;
        return {
            constraints.isInlineTight() ? constraints.maxInline : preferredWidth(heightConstraint),
            constraints.isBlockTight() ? constraints.maxBlock : preferredHeight(widthConstraint),
        };
    }

    AMinMaxAxis onComputeIntrinsicMinMaxAxis(int) override {
        return {
            .min = {},
            .max = preferredWidth(-1),
        };
    }
};

class WrappingMinMaxTestView : public AView {
public:
    AMinMaxAxis onComputeIntrinsicMinMaxAxis(int) override {
        return {
            .min = 20,
            .max = 80,
        };
    }

    glm::ivec2 onIntrinsicMeasure(AConstraints constraints) override {
        const int width = constraints.isInlineTight() ? constraints.maxInline : 80;
        return {
            width,
            width <= 20 ? 50 : 10,
        };
    }
};

class SplitterMeasureView : public AView {
public:
    int preferredWidth = 0;
    int preferredHeight = 0;

    glm::ivec2 onIntrinsicMeasure(AConstraints constraints) override {
        return {
            constraints.maxInline,
            constraints.isUnlimitedBlock() ? preferredHeight : constraints.maxBlock,
        };
    }

    AMinMaxAxis onComputeIntrinsicMinMaxAxis(int) override {
        return {
            .min = {},
            .max = preferredWidth,
        };
    }
};

template<typename T>
void expectRect(const _<T>& item, glm::ivec2 position, glm::ivec2 size) {
    EXPECT_EQ(item->getPosition(), position);
    EXPECT_EQ(item->getSize(), size);
}

}   // namespace

TEST(ASplitterHelper, ReclaimSpaceUsesViewMinimumSize) {
    auto view = _new<SplitterMeasureView>();
    view->preferredWidth = 80;
    view->preferredHeight = 10;
    view->setMinSize({ 30, 0 });
    view->setSize({ 100, 20 });

    ASplitterHelper helper(ALayoutDirection::HORIZONTAL);
    helper.setItems(AVector<ASplitterHelper::Item> { { .view = view } });

    EXPECT_EQ(helper.reclaimSpace(100, 0), 30);
    ASSERT_TRUE(helper.items().first().overridedSize.hasValue());
    EXPECT_EQ(*helper.items().first().overridedSize, 30);
}

TEST(ASplitterHelper, OverridenSizeIsExactOnMainAxis) {
    auto view = _new<FakeLayoutItem>();
    view->preferredWidth = [](int) { return 80; };
    view->preferredHeight = [](int) { return 10; };
    view->setMinSize({ 7, 0 });
    view->setExpanding({ 1, 0 });

    ASplitterHelper::Item item {
        .view = view,
        .overridedSize = 20,
    };

    SizeInjector<ALayoutDirection::HORIZONTAL> injected { item };

    EXPECT_EQ(injected.computeMinMaxAxis().max, 20);
    EXPECT_EQ(injected.computeMinMaxAxis().min, 20);
    EXPECT_EQ(injected.getFixedSize().x, 0);
    EXPECT_EQ(injected.getMinSize().x, 7);
    EXPECT_EQ(injected.getExpanding().x, 0);
}

TEST(ASplitterHelper, OverridenSizeDoesNotGoBelowViewMinimum) {
    auto view = _new<FakeLayoutItem>();
    view->preferredWidth = [](int) { return 80; };
    view->preferredHeight = [](int) { return 10; };
    view->setMinSize({ 30, 0 });

    ASplitterHelper::Item item {
        .view = view,
        .overridedSize = 20,
    };

    SizeInjector<ALayoutDirection::HORIZONTAL> injected { item };

    EXPECT_EQ(injected.computeMinMaxAxis().max, 30);
    EXPECT_EQ(injected.computeMinMaxAxis().min, 30);
}

TEST(ASplitterHelper, OverridenSizeMasksFixedSizeOnMainAxis) {
    auto view = _new<FakeLayoutItem>();
    view->setFixedSize({ 80, 10 });

    ASplitterHelper::Item item {
        .view = view,
        .overridedSize = 20,
    };

    SizeInjector<ALayoutDirection::HORIZONTAL> injected { item };

    EXPECT_EQ(injected.getFixedSize().x, 0);
    EXPECT_EQ(injected.getFixedSize().y, 10);
    EXPECT_EQ(injected.computeMinMaxAxis().max, 20);
    EXPECT_EQ(injected.computeMinMaxAxis().min, 20);
}

TEST(ASplitter, DefaultsToEqualHorizontalExpansionWhenChildrenAreNotExpanding) {
    auto left = _new<FakeLayoutItem>();
    left->preferredWidth = [](int) { return 40; };
    left->preferredHeight = [](int) { return 10; };

    auto right = _new<FakeLayoutItem>();
    right->preferredWidth = [](int) { return 40; };
    right->preferredHeight = [](int) { return 10; };

    auto splitter = _cast<ASplitter>(ASplitter::Horizontal()
        .withItems({ left, right })
        .build());

    splitter->layout(0, 0, 100, 20);

    expectRect(left, { 0, 0 }, { 50, 10 });
    expectRect(right, { 50, 0 }, { 50, 10 });
}

TEST(ASplitter, HorizontalMinHeightUsesChildHeightAtMinimumWidths) {
    auto wrapping = _new<WrappingMinMaxTestView>();
    auto fixed = _new<FakeLayoutItem>();
    fixed->setMinSize({ 20, 10 });
    fixed->preferredWidth = [](int) { return 20; };
    fixed->preferredHeight = [](int) { return 10; };

    auto splitter = _cast<ASplitter>(ASplitter::Horizontal()
        .withItems({ wrapping, fixed })
        .build());

    const auto minMax = splitter->computeMinMaxAxis();

    EXPECT_EQ(minMax.min, 40);
}
