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

#include <gtest/gtest.h>
#include <range/v3/range.hpp>
#include <AUI/Layout/HVLayout.h>
#include <AUI/View/AView.h>
#include <AUI/View/ASplitter.h>
#include <AUI/View/ASplitterHelper.h>
#include <AUI/Util/SplitterSizeInjector.h>
#include <functional>
#include <limits>

template<typename T>
inline std::ostream& operator<<(std::ostream& o, const glm::tvec2<T>& v) {
    return o << "{ " << v.x << ", " << v.y << " }";
}

namespace {

using HorizontalHVLayout = aui::HVLayout<ALayoutDirection::HORIZONTAL>;
using VerticalHVLayout = aui::HVLayout<ALayoutDirection::VERTICAL>;

class FakeLayoutItem : public AView {
public:
    std::function<int(int)> preferredWidth = [](int) { return 0; };
    std::function<int(int)> preferredHeight = [](int) { return 0; };

    int onComputeIntrinsicWidth(int height) override {
        return preferredWidth(height);
    }

    int onComputeIntrinsicHeight(int width) override {
        return preferredHeight(width);
    }
};

class MeasureTestView : public AView {
public:
    int intrinsicWidth = 0;
    int intrinsicHeight = 0;
    glm::ivec2 measuredContentSize = {};
    int lastWidthConstraint = std::numeric_limits<int>::min();
    int lastHeightConstraint = std::numeric_limits<int>::min();
    AConstraints lastMeasureConstraints {};

    int onComputeIntrinsicWidth(int height) override {
        lastHeightConstraint = height;
        return intrinsicWidth;
    }

    int onComputeIntrinsicHeight(int width) override {
        lastWidthConstraint = width;
        return intrinsicHeight;
    }

    glm::ivec2 onIntrinsicMeasure(AConstraints constraints) override {
        lastMeasureConstraints = constraints;
        return measuredContentSize;
    }
};

class SplitterMeasureView : public AView {
public:
    int preferredWidth = 0;
    int preferredHeight = 0;

    int onComputeIntrinsicWidth(int height) override {
        return preferredWidth;
    }

    int onComputeIntrinsicHeight(int width) override {
        return preferredHeight;
    }

    glm::ivec2 onIntrinsicMeasure(AConstraints constraints) override {
        return {
            constraints.maxWidth,
            constraints.maxHeight == 1000000 ? preferredHeight : constraints.maxHeight,
        };
    }
};

template<typename T>
void expectRect(const _<T>& item, glm::ivec2 position, glm::ivec2 size) {
    EXPECT_EQ(item->getPosition(), position);
    EXPECT_EQ(item->getSize(), size);
}

}   // namespace

TEST(HVLayout, HorizontalExpandingConsumesRemainingSpace) {
    auto leading = _new<FakeLayoutItem>();
    leading->setFixedSize({ 24, 24 });

    auto spacer = _new<FakeLayoutItem>();
    spacer->setExpanding({ 1, 1 });

    auto trailing = _new<FakeLayoutItem>();
    trailing->setFixedSize({ 24, 24 });

    AVector<_<AView>> views { leading, spacer, trailing };
    HorizontalHVLayout::layout(glm::ivec2{ 0, 0 }, glm::ivec2{ 100, 24 }, views, 0);

    expectRect(leading, { 0, 0 }, { 24, 24 });
    expectRect(spacer, { 24, 0 }, { 52, 24 });
    expectRect(trailing, { 76, 0 }, { 24, 24 });
}

TEST(HVLayout, HorizontalFixedExpandingFixedKeepsTrailingChildInside) {
    auto leading = _new<FakeLayoutItem>();
    leading->setFixedSize({ 24, 24 });

    auto middle = _new<FakeLayoutItem>();
    middle->setExpanding({ 1, 0 });
    middle->preferredWidth = [](int) { return 200; };
    middle->preferredHeight = [](int) { return 12; };

    auto trailing = _new<FakeLayoutItem>();
    trailing->setFixedSize({ 24, 24 });

    AVector<_<AView>> views { leading, middle, trailing };
    HorizontalHVLayout::layout({ 0, 0 }, { 120, 24 }, views, 0);

    expectRect(leading, { 0, 0 }, { 24, 24 });
    expectRect(middle, { 24, 0 }, { 72, 12 });
    expectRect(trailing, { 96, 0 }, { 24, 24 });
}

TEST(HVLayout, HorizontalIntrinsicHeightUsesAllocatedWidthForExpandingChildren) {
    auto fixed = _new<FakeLayoutItem>();
    fixed->setFixedSize({ 20, 10 });

    auto wrapped = _new<FakeLayoutItem>();
    wrapped->setExpanding({ 1, 0 });
    wrapped->preferredWidth = [](int) { return 60; };
    wrapped->preferredHeight = [](int width) {
        if (width == -1) {
            return 10;
        }
        return width <= 20 ? 40 : 10;
    };

    AVector<_<AView>> views { fixed, wrapped };

    EXPECT_EQ(HorizontalHVLayout::onComputeIntrinsicHeight(views, 0, -1), 10);
    EXPECT_EQ(HorizontalHVLayout::onComputeIntrinsicHeight(views, 0, 40), 40);
}

TEST(HVLayout, HorizontalIntrinsicWidthUsesPreferredSizeForExpandingChildren) {
    auto fixed = _new<FakeLayoutItem>();
    fixed->setFixedSize({ 24, 24 });

    auto expanding = _new<FakeLayoutItem>();
    expanding->setExpanding({ 1, 0 });
    expanding->setMinSize({ 10, 0 });
    expanding->preferredWidth = [](int) { return 200; };
    expanding->preferredHeight = [](int) { return 12; };

    auto spacer = _new<FakeLayoutItem>();
    spacer->setExpanding({ 4, 0 });

    AVector<_<AView>> views { fixed, expanding, spacer };

    EXPECT_EQ(HorizontalHVLayout::onComputeIntrinsicWidth(views, 0, -1), 224);
}

TEST(HVLayout, VerticalExpandingConsumesRemainingSpace) {
    auto top = _new<FakeLayoutItem>();
    top->setFixedSize({ 24, 24 });

    auto middle = _new<FakeLayoutItem>();
    middle->setExpanding({ 1, 1 });

    auto bottom = _new<FakeLayoutItem>();
    bottom->setFixedSize({ 24, 24 });

    AVector<_<AView>> views { top, middle, bottom };
    VerticalHVLayout::layout({ 0, 0 }, { 24, 100 }, views, 0);

    expectRect(top, { 0, 0 }, { 24, 24 });
    expectRect(middle, { 0, 24 }, { 24, 52 });
    expectRect(bottom, { 0, 76 }, { 24, 24 });
}

TEST(AViewMeasure, ComputeWidthAndHeightUsePaddingAdjustedConstraints) {
    MeasureTestView view;
    view.setPadding({ .left = 3, .right = 5, .top = 7, .bottom = 11 });
    view.intrinsicWidth = 10;
    view.intrinsicHeight = 9;

    EXPECT_EQ(view.computeWidth(40), 18);
    EXPECT_EQ(view.lastHeightConstraint, 22);

    EXPECT_EQ(view.computeHeight(50), 27);
    EXPECT_EQ(view.lastWidthConstraint, 42);
}

TEST(AViewMeasure, MeasurePassesContentConstraintsAndAddsPaddingBack) {
    MeasureTestView view;
    view.setPadding({ .left = 3, .right = 5, .top = 7, .bottom = 11 });
    view.measuredContentSize = { 20, 10 };

    auto measured = view.measure({
        .minWidth = 30,
        .minHeight = 25,
        .maxWidth = 40,
        .maxHeight = 60,
    });

    EXPECT_EQ(view.lastMeasureConstraints.minWidth, 22);
    EXPECT_EQ(view.lastMeasureConstraints.maxWidth, 32);
    EXPECT_EQ(view.lastMeasureConstraints.minHeight, 7);
    EXPECT_EQ(view.lastMeasureConstraints.maxHeight, 42);
    EXPECT_EQ(measured, glm::ivec2(30, 28));
}

TEST(AViewMeasure, MeasureHonorsMinAndMaxSize) {
    MeasureTestView view;
    view.setMinSize({ 20, 30 });
    view.setMaxSize({ 40, 50 });
    view.measuredContentSize = { 10, 60 };

    auto measured = view.measure(AConstraints {});

    EXPECT_EQ(view.lastMeasureConstraints.minWidth, 20);
    EXPECT_EQ(view.lastMeasureConstraints.maxWidth, 40);
    EXPECT_EQ(view.lastMeasureConstraints.minHeight, 30);
    EXPECT_EQ(view.lastMeasureConstraints.maxHeight, 50);
    EXPECT_EQ(measured, glm::ivec2(20, 50));
}

TEST(AViewMeasure, MeasureHonorsFixedSizeAndContentConstraints) {
    MeasureTestView view;
    view.setPadding({ .left = 4, .right = 6, .top = 2, .bottom = 8 });
    view.setFixedSize({ 50, 30 });
    view.measuredContentSize = { 0, 0 };

    auto measured = view.measure(AConstraints {});

    EXPECT_EQ(view.lastMeasureConstraints.minWidth, 40);
    EXPECT_EQ(view.lastMeasureConstraints.maxWidth, 40);
    EXPECT_EQ(view.lastMeasureConstraints.minHeight, 20);
    EXPECT_EQ(view.lastMeasureConstraints.maxHeight, 20);
    EXPECT_EQ(measured, glm::ivec2(50, 30));
}

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

    EXPECT_EQ(injected.computeWidth(-1), 20);
    EXPECT_EQ(injected.computeIntrinsicWidth(-1), 20);
    EXPECT_EQ(injected.getFixedSize().x, 0);
    EXPECT_EQ(injected.getMinimumSize().x, 7);
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

    EXPECT_EQ(injected.computeWidth(-1), 30);
    EXPECT_EQ(injected.computeIntrinsicWidth(-1), 30);
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
