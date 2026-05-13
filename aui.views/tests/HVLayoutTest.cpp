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

#include "AUI/ASS/Property/MinSize.h"

#include <gtest/gtest.h>
#include <AUI/Layout/HVLayout.h>
#include <AUI/View/AView.h>
#include <functional>
#include <utility>

template<typename T>
inline std::ostream& operator<<(std::ostream& o, const glm::tvec2<T>& v) {
    return o << "{ " << v.x << ", " << v.y << " }";
}

namespace {

using HorizontalHVLayout = aui::HVLayout<ALayoutDirection::HORIZONTAL>;
using VerticalHVLayout = aui::HVLayout<ALayoutDirection::VERTICAL>;

glm::ivec2 clampMeasuredSize(AConstraints constraints, glm::ivec2 size) {
    if (!constraints.isUnlimitedInline()) {
        size.x = std::min(size.x, constraints.maxInline);
    }
    if (!constraints.isUnlimitedBlock()) {
        size.y = std::min(size.y, constraints.maxBlock);
    }
    size.x = std::max(size.x, constraints.minInline);
    size.y = std::max(size.y, constraints.minBlock);
    return size;
}

class FixedLayoutItem : public AView {
public:
    FixedLayoutItem(int preferredWidth, int preferredHeight)
        : mPreferredSize(preferredWidth, preferredHeight) {}

    glm::ivec2 onIntrinsicMeasure(AConstraints constraints) override {
        return clampMeasuredSize(constraints, {
            constraints.isInlineTight() ? constraints.maxInline : mPreferredSize.x,
            constraints.isBlockTight() ? constraints.maxBlock : mPreferredSize.y,
        });
    }

    AMinMaxAxis onComputeIntrinsicMinMaxAxis(int) override {
        return {
            .min = {},
            .max = mPreferredSize.x,
        };
    }

private:
    glm::ivec2 mPreferredSize;
};

class DynamicLayoutItem : public AView {
public:
    using AxisFunction = std::function<int(int)>;
    using IntrinsicWidthFunction = std::function<AMinMaxAxis(int)>;

    DynamicLayoutItem(
        AxisFunction preferredWidth,
        AxisFunction preferredHeight,
        IntrinsicWidthFunction intrinsicWidth = {})
        : mPreferredWidth(std::move(preferredWidth)),
          mPreferredHeight(std::move(preferredHeight)),
          mIntrinsicWidth(std::move(intrinsicWidth)) {}

    glm::ivec2 onIntrinsicMeasure(AConstraints constraints) override {
        const int widthConstraint = constraints.isUnlimitedInline() ? -1 : constraints.maxInline;
        const int heightConstraint = constraints.isUnlimitedBlock() ? -1 : constraints.maxBlock;
        return clampMeasuredSize(constraints, {
            constraints.isInlineTight() ? constraints.maxInline : mPreferredWidth(heightConstraint),
            constraints.isBlockTight() ? constraints.maxBlock : mPreferredHeight(widthConstraint),
        });
    }

    AMinMaxAxis onComputeIntrinsicMinMaxAxis(int height) override {
        if (mIntrinsicWidth) {
            return mIntrinsicWidth(height);
        }
        return {
            .min = {},
            .max = mPreferredWidth(height),
        };
    }

private:
    AxisFunction mPreferredWidth;
    AxisFunction mPreferredHeight;
    IntrinsicWidthFunction mIntrinsicWidth;
};

class ZeroMainAxisMinView : public AView {
public:
    int minWidth = 0;
    int preferredHeight = 10;

    glm::ivec2 onIntrinsicMeasure(AConstraints constraints) override {
        if (constraints.isBlockTight() && constraints.maxBlock == 0) {
            return { 100000, 0 };
        }
        int w = minWidth;
        int h = constraints.isBlockTight() ? constraints.maxBlock : preferredHeight;
        if (!constraints.isUnlimitedInline()) w = std::min(w, constraints.maxInline);
        if (!constraints.isUnlimitedBlock()) h = std::min(h, constraints.maxBlock);
        return { w, h };
    }

    AMinMaxAxis onComputeIntrinsicMinMaxAxis(int) override {
        return {
            .min = minWidth,
            .max = minWidth,
        };
    }
};

class PreferredButMinConstrainedView : public AView {
public:
    PreferredButMinConstrainedView(int preferredWidth, int preferredHeight, int minWidth)
        : mPreferredWidth(preferredWidth), mPreferredHeight(preferredHeight) {
        setMinSize({ minWidth, 0 });
      setCustomStyle({
        ass::MinSize { AMetric(minWidth, AMetric::T_PX), 0_px }
      });
    }

    glm::ivec2 onIntrinsicMeasure(AConstraints constraints) override {
        int w = constraints.isInlineTight() ? constraints.maxInline : mPreferredWidth;
        int h = constraints.isBlockTight() ? constraints.maxBlock : mPreferredHeight;
        if (!constraints.isUnlimitedInline()) w = std::min(w, constraints.maxInline);
        if (!constraints.isUnlimitedBlock()) h = std::min(h, constraints.maxBlock);
        if (w < constraints.minInline) w = constraints.minInline;
        if (h < constraints.minBlock) h = constraints.minBlock;
        return { w, h };
    }

    AMinMaxAxis onComputeIntrinsicMinMaxAxis(int) override {
        return {
            .min = {},
            .max = mPreferredWidth,
        };
    }

private:
    int mPreferredWidth;
    int mPreferredHeight;
};

_<FixedLayoutItem> fixedItem(int preferredWidth, int preferredHeight) {
    return _new<FixedLayoutItem>(preferredWidth, preferredHeight);
}

template<typename PreferredWidth, typename PreferredHeight>
_<DynamicLayoutItem> dynamicItem(PreferredWidth preferredWidth, PreferredHeight preferredHeight) {
    return _new<DynamicLayoutItem>(std::move(preferredWidth), std::move(preferredHeight));
}

template<typename PreferredWidth, typename PreferredHeight, typename IntrinsicWidth>
_<DynamicLayoutItem> dynamicItem(
    PreferredWidth preferredWidth,
    PreferredHeight preferredHeight,
    IntrinsicWidth intrinsicWidth) {
    return _new<DynamicLayoutItem>(
        std::move(preferredWidth),
        std::move(preferredHeight),
        std::move(intrinsicWidth));
}

}   // namespace

#define EXPECT_RECT(item, x, y, width, height) \
    do { \
        const auto expectedPosition = glm::ivec2{ (x), (y) }; \
        const auto expectedSize = glm::ivec2{ (width), (height) }; \
        EXPECT_EQ((item)->getPosition(), expectedPosition); \
        EXPECT_EQ((item)->getSize(), expectedSize); \
    } while (false)

TEST(HVLayout, HorizontalExpandingConsumesRemainingSpace) {
    auto leading = fixedItem(24, 24);

    auto spacer = fixedItem(0, 0);
    spacer->setExpanding({ 1, 1 });

    auto trailing = fixedItem(24, 24);

    AVector<_<AView>> views { leading, spacer, trailing };
    HorizontalHVLayout::layout(glm::ivec2{ 0, 0 }, glm::ivec2{ 100, 24 }, views, 0);

    EXPECT_RECT(leading, 0, 0, 24, 24);
    EXPECT_RECT(spacer, 24, 0, 52, 24);
    EXPECT_RECT(trailing, 76, 0, 24, 24);
}

TEST(HVLayout, HorizontalFixedExpandingFixedKeepsTrailingChildInside) {
    auto leading = fixedItem(24, 24);

    auto middle = fixedItem(200, 12);
    middle->setExpanding({ 1, 0 });

    auto trailing = fixedItem(24, 24);

    AVector<_<AView>> views { leading, middle, trailing };
    HorizontalHVLayout::layout({ 0, 0 }, { 120, 24 }, views, 0);

    EXPECT_RECT(leading, 0, 0, 24, 24);
    EXPECT_RECT(middle, 24, 0, 72, 12);
    EXPECT_RECT(trailing, 96, 0, 24, 24);
}

TEST(HVLayout, HorizontalIntrinsicHeightUsesAllocatedWidthForExpandingChildren) {
    auto fixed = fixedItem(20, 10);

    auto wrapped = dynamicItem(
        [](int) { return 60; },
        [](int width) {
            if (width == -1) {
                return 10;
            }
            return width <= 20 ? 40 : 10;
        });
    wrapped->setExpanding({ 1, 0 });

    AVector<_<AView>> views { fixed, wrapped };

    EXPECT_EQ(HorizontalHVLayout::preferredHeight(views, 0, -1), 10);
    EXPECT_EQ(HorizontalHVLayout::preferredHeight(views, 0, 40), 40);
}

TEST(HVLayout, HorizontalIntrinsicWidthUsesPreferredSizeForExpandingChildren) {
    auto fixed = fixedItem(24, 24);

    auto expanding = fixedItem(200, 12);
    expanding->setExpanding({ 1, 0 });
    expanding->setMinSize({ 10, 0 });

    auto spacer = fixedItem(0, 0);
    spacer->setExpanding({ 4, 0 });

    AVector<_<AView>> views { fixed, expanding, spacer };

    EXPECT_EQ(HorizontalHVLayout::preferredWidth(views, 0, -1), 224);
}

TEST(HVLayout, HorizontalIntrinsicHeightTightWidthSkipsGoneExpander) {
    auto fixed = fixedItem(20, 10);

    auto gone = fixedItem(100, 100);
    gone->setExpanding({ 10, 0 });
    gone->setVisibility(Visibility::GONE);

    auto wrapped = dynamicItem(
        [](int) { return 60; },
        [](int width) {
            if (width == -1) {
                return 10;
            }
            return width <= 20 ? 40 : 10;
        });
    wrapped->setExpanding({ 1, 0 });

    AVector<_<AView>> views { fixed, gone, wrapped };

    EXPECT_EQ(HorizontalHVLayout::preferredHeight(views, 0, 40), 40);
}

TEST(HVLayout, HorizontalComputeMinMaxSizesUsesChildrenAndSpacing) {
    auto left = fixedItem(30, 20);
    left->setMinSize({ 10, 5 });
    left->setMargin({ .right = 3, .bottom = 2 });

    auto right = fixedItem(40, 13);
    right->setMinSize({ 7, 11 });
    right->setMargin({ .left = 1, .top = 4 });

    AVector<_<AView>> views { left, right };
    const auto minMax = HorizontalHVLayout::computeIntrinsicMinMaxSizes(views, 6);

    EXPECT_EQ(minMax.min, 27);
    EXPECT_EQ(minMax.max, 80);
}

TEST(HVLayout, HorizontalComputeMinMaxSizesSkipsGoneChildrenAndSpacing) {
    auto left = fixedItem(20, 10);
    left->setMinSize({ 5, 0 });

    auto gone = fixedItem(100, 10);
    gone->setVisibility(Visibility::GONE);
    gone->setMargin({ .left = 30, .right = 40 });

    auto right = fixedItem(30, 10);
    right->setMinSize({ 7, 0 });

    AVector<_<AView>> views { left, gone, right };
    const auto minMax = HorizontalHVLayout::computeIntrinsicMinMaxSizes(views, 9);

    EXPECT_EQ(minMax.min, 21);
    EXPECT_EQ(minMax.max, 59);
}

TEST(HVLayout, VerticalExpandingConsumesRemainingSpace) {
    auto top = fixedItem(24, 24);

    auto middle = fixedItem(0, 0);
    middle->setExpanding({ 1, 1 });

    auto bottom = fixedItem(24, 24);

    AVector<_<AView>> views { top, middle, bottom };
    VerticalHVLayout::layout({ 0, 0 }, { 24, 100 }, views, 0);

    EXPECT_RECT(top, 0, 0, 24, 24);
    EXPECT_RECT(middle, 0, 24, 24, 52);
    EXPECT_RECT(bottom, 0, 76, 24, 24);
}

TEST(HVLayout, VerticalComputeMinWidthIgnoresZeroHeightProbeGarbage) {
    auto narrow = _new<ZeroMainAxisMinView>();
    narrow->minWidth = 50;

    auto wide = _new<ZeroMainAxisMinView>();
    wide->minWidth = 3173;

    AVector<_<AView>> views { narrow, wide };

    const auto minMax = VerticalHVLayout::computeIntrinsicMinMaxSizes(views, 0);

    EXPECT_EQ(minMax.min, 3173);
}

TEST(HVLayout, HorizontalSpacingAndMargins) {
    auto left = fixedItem(20, 20);
    left->setMargin({ .right = 10 });

    auto right = fixedItem(20, 20);
    right->setMargin({ .left = 5 });

    AVector<_<AView>> views { left, right };
    HorizontalHVLayout::layout({ 0, 0 }, { 100, 20 }, views, 15);

    EXPECT_RECT(left, 0, 0, 20, 20);
    // left_pos(0) + left_size(20) + left_margin_right(10) + spacing(15) + right_margin_left(5) = 50
    EXPECT_RECT(right, 50, 0, 20, 20);
}

TEST(HVLayout, HorizontalExpandingBetweenFixedViewsWithSpacingAndMargins) {
    auto leading = fixedItem(10, 20);
    leading->setMargin({ .right = 3 });

    auto middle = fixedItem(0, 20);
    middle->setExpanding({ 1, 0 });
    middle->setMargin({ .left = 4, .right = 5 });

    auto trailing = fixedItem(10, 20);
    trailing->setMargin({ .left = 2 });

    AVector<_<AView>> views { leading, middle, trailing };
    HorizontalHVLayout::layout({ 0, 0 }, { 100, 20 }, views, 7);

    EXPECT_RECT(leading, 0, 0, 10, 20);
    EXPECT_RECT(middle, 24, 0, 52, 20);
    EXPECT_RECT(trailing, 90, 0, 10, 20);
}

TEST(HVLayout, HorizontalVisibility) {
    auto v1 = fixedItem(20, 20);

    auto v2 = fixedItem(20, 20);
    v2->setVisibility(Visibility::GONE);

    auto v3 = fixedItem(20, 20);
    v3->setVisibility(Visibility::INVISIBLE);

    AVector<_<AView>> views { v1, v2, v3 };
    HorizontalHVLayout::layout({ 0, 0 }, { 100, 20 }, views, 10);

    EXPECT_RECT(v1, 0, 0, 20, 20);
    // v2 is GONE, so it should be skipped.
    // v3 is INVISIBLE, but it has CONSUME_SPACE, so it should be placed.
    EXPECT_RECT(v3, 30, 0, 20, 20);
}

TEST(HVLayout, VerticalVisibility) {
    auto v1 = fixedItem(20, 20);

    auto v2 = fixedItem(20, 20);
    v2->setVisibility(Visibility::GONE);

    auto v3 = fixedItem(20, 20);
    v3->setVisibility(Visibility::INVISIBLE);

    AVector<_<AView>> views { v1, v2, v3 };
    VerticalHVLayout::layout({ 0, 0 }, { 20, 100 }, views, 10);

    EXPECT_RECT(v1, 0, 0, 20, 20);
    EXPECT_RECT(v3, 0, 30, 20, 20);
}

TEST(HVLayout, HorizontalGoneExpandingViewDoesNotReceiveSpace) {
    auto leading = fixedItem(20, 20);

    auto gone = fixedItem(0, 20);
    gone->setExpanding({ 10, 0 });
    gone->setVisibility(Visibility::GONE);

    auto middle = fixedItem(0, 20);
    middle->setExpanding({ 1, 0 });

    auto trailing = fixedItem(20, 20);

    AVector<_<AView>> views { leading, gone, middle, trailing };
    HorizontalHVLayout::layout({ 0, 0 }, { 100, 20 }, views, 0);

    EXPECT_RECT(leading, 0, 0, 20, 20);
    EXPECT_RECT(middle, 20, 0, 60, 20);
    EXPECT_RECT(trailing, 80, 0, 20, 20);
}

TEST(HVLayout, HorizontalWeightSaturation) {
    auto e1 = fixedItem(0, 20);
    e1->setExpanding({ 1, 0 });
    e1->setMaxSize({ 40, -1 });

    auto e2 = fixedItem(0, 20);
    e2->setExpanding({ 1, 0 });

    AVector<_<AView>> views { e1, e2 };
    // Total width 100. e1 and e2 both want 50. But e1 is capped at 40.
    // So e2 should get 100 - 40 = 60.
    HorizontalHVLayout::layout({ 0, 0 }, { 100, 20 }, views, 0);

    EXPECT_RECT(e1, 0, 0, 40, 20);
    EXPECT_RECT(e2, 40, 0, 60, 20);
}

TEST(HVLayout, HorizontalSingleExpandingChildRespectsMaxSize) {
    auto only = fixedItem(0, 20);
    only->setExpanding({ 1, 0 });
    only->setMaxSize({ 30, -1 });

    AVector<_<AView>> views { only };
    HorizontalHVLayout::layout({ 0, 0 }, { 100, 20 }, views, 0);

    EXPECT_RECT(only, 0, 0, 30, 20);
}

TEST(HVLayout, HorizontalWeightSaturationComplex) {
    auto e1 = fixedItem(0, 20);
    e1->setExpanding({ 1, 0 });
    e1->setMaxSize({ 20, -1 });

    auto e2 = fixedItem(0, 20);
    e2->setExpanding({ 2, 0 });
    e2->setMaxSize({ 40, -1 });

    auto e3 = fixedItem(0, 20);
    e3->setExpanding({ 1, 0 });

    AVector<_<AView>> views { e1, e2, e3 };
    // Total 100. Weights 1:2:1. Shares: 25, 50, 25.
    // e1 capped at 20. Remaining 80. Weights 2:1. Shares 53, 26.
    // e2 capped at 40. Remaining 40. Weight 1. Share 40.
    // Final: 20, 40, 40.
    HorizontalHVLayout::layout({ 0, 0 }, { 100, 20 }, views, 0);

    EXPECT_RECT(e1, 0, 0, 20, 20);
    EXPECT_RECT(e2, 20, 0, 40, 20);
    EXPECT_RECT(e3, 60, 0, 40, 20);
}

TEST(HVLayout, HorizontalExpandingUsesMinimumBeforeWeightDistribution) {
    auto left = fixedItem(0, 20);
    left->setExpanding({ 1, 0 });
    left->setMinSize({ 20, 0 });

    auto right = fixedItem(0, 20);
    right->setExpanding({ 1, 0 });
    right->setMinSize({ 10, 0 });

    AVector<_<AView>> views { left, right };
    HorizontalHVLayout::layout({ 0, 0 }, { 90, 20 }, views, 0);

    EXPECT_RECT(left, 0, 0, 50, 20);
    EXPECT_RECT(right, 50, 0, 40, 20);
}

TEST(HVLayout, VerticalSpacingAndMargins) {
    auto top = fixedItem(20, 20);
    top->setMargin({ .bottom = 10 });

    auto bottom = fixedItem(20, 20);
    bottom->setMargin({ .top = 5 });

    AVector<_<AView>> views { top, bottom };
    VerticalHVLayout::layout({ 0, 0 }, { 20, 100 }, views, 15);

    EXPECT_RECT(top, 0, 0, 20, 20);
    EXPECT_RECT(bottom, 0, 50, 20, 20);
}

TEST(HVLayout, VerticalExpandingBetweenFixedViewsWithSpacingAndMargins) {
    auto top = fixedItem(20, 10);
    top->setMargin({ .bottom = 3 });

    auto middle = fixedItem(20, 0);
    middle->setExpanding({ 0, 1 });
    middle->setMargin({ .top = 4, .bottom = 5 });

    auto bottom = fixedItem(20, 10);
    bottom->setMargin({ .top = 2 });

    AVector<_<AView>> views { top, middle, bottom };
    VerticalHVLayout::layout({ 0, 0 }, { 20, 100 }, views, 7);

    EXPECT_RECT(top, 0, 0, 20, 10);
    EXPECT_RECT(middle, 0, 24, 20, 52);
    EXPECT_RECT(bottom, 0, 90, 20, 10);
}

TEST(HVLayout, VerticalLayoutUsesWidthFromFinalHeight) {
    auto child = _new<PreferredButMinConstrainedView>(120, 20, 60);

    AVector<_<AView>> views { child };
    VerticalHVLayout::layout({ 0, 0 }, { 40, 100 }, views, 0);

    EXPECT_RECT(child, 0, 0, 60, 20);
}

TEST(HVLayout, PerpendicularContribution) {
    auto item = dynamicItem(
        [](int) { return 100; },
        [](int width) { return width == 10 ? 50 : 20; });
    item->setMinSize({ 10, 10 });
    item->setMargin({ .top = 2, .bottom = 3, .left = 4, .right = 1 });

    AVector<_<AView>> views { item };

    // Horizontal: min our is 10. perp at 10 is 50. + margin(2+3) = 55.
    EXPECT_EQ(HorizontalHVLayout::computeMinimumPerpendicularContribution(item, item->getMargin().occupiedSize()), 55);
    // Horizontal: max our is 100. perp at 100 is 20. + margin(2+3) = 25.
    EXPECT_EQ(HorizontalHVLayout::computeMaximumPerpendicularContribution(item, item->getMargin().occupiedSize()), 25);

    // Vertical: min our is 10. + margin(2+3) = 15.
    EXPECT_EQ(VerticalHVLayout::computeMinimumPerpendicularContribution(item, item->getMargin().occupiedSize()), 15);
    // Vertical: max our is 100 + margin(4+1) = 105.
    EXPECT_EQ(VerticalHVLayout::computeMaximumPerpendicularContribution(item, item->getMargin().occupiedSize()), 105);
}

TEST(HVLayout, HorizontalPerpendicularMaxSizeCapsExpandingHeight) {
    auto item = fixedItem(40, 10);
    item->setExpanding({ 0, 1 });
    item->setMaxSize({ -1, 15 });

    AVector<_<AView>> views { item };
    HorizontalHVLayout::layout({ 0, 0 }, { 40, 30 }, views, 0);

    EXPECT_RECT(item, 0, 0, 40, 15);
}

TEST(HVLayout, VerticalPerpendicularMaxSizeCapsExpandingWidth) {
    auto item = fixedItem(10, 40);
    item->setExpanding({ 1, 0 });
    item->setMaxSize({ 15, -1 });

    AVector<_<AView>> views { item };
    VerticalHVLayout::layout({ 0, 0 }, { 30, 40 }, views, 0);

    EXPECT_RECT(item, 0, 0, 15, 40);
}

TEST(HVLayout, VerticalExpandingChildDoesNotOverflow) {
    auto top = fixedItem(0, 27);
    top->setExpanding({ 1, 0 }); // horizontal expanding, fixed height

    auto bottom = dynamicItem(
        [](int height) {
            if (height != -1 && height <= 700) return 512;
            return 1377;
        },
        [](int) { return 1174; });
    bottom->setExpanding({ 1, 1 }); // all directions
    // Simulate the user's view behavior:
    // It wants to be 1174 high if unconstrained, but can be as small as 512 if constrained.

    AVector<_<AView>> views { top, bottom };
    // Container 700x700
    VerticalHVLayout::layout({ 0, 0 }, { 700, 700 }, views, 0);

    EXPECT_RECT(top, 0, 0, 700, 27);
    // The height of 'bottom' should be 700 - 27 = 673.
    // If it's 1174, the test fails.
    EXPECT_EQ(bottom->getSize().y, 673);
}

TEST(HVLayout, VerticalMeasurementConsistency) {
    auto child = fixedItem(143, 10);

    AVector<_<AView>> views { child };

    // This should return 143 as the max width
    const auto minMax = VerticalHVLayout::computeIntrinsicMinMaxSizes(views, 0);
    EXPECT_EQ(minMax.min, 0);
    EXPECT_EQ(minMax.max, 143);

    // When measured with 512, it should still respect its intrinsic max of 143
    AConstraints constraints;
    constraints.maxInline = 512;
    constraints.maxBlock = 512;
    const auto measured = VerticalHVLayout::onIntrinsicMeasure(views, 0, constraints);
    EXPECT_EQ(measured.x, 143);
}

#undef EXPECT_RECT
