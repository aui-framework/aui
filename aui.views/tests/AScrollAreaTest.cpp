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
#include <AUI/ASS/ASS.h>
#include <AUI/Util/AMetric.h>
#include <AUI/View/AScrollArea.h>
#include <AUI/View/AScrollAreaViewport.h>
#include <AUI/View/AScrollbar.h>

namespace {

class TrackingWrappingView : public AView {
public:
    glm::ivec2 onIntrinsicMeasure(AConstraints constraints) override {
        const int width = constraints.isUnlimitedInline() ? 200 : constraints.maxInline;
        mLastMeasuredWidth = width;
        if (width > 100) {
            return { width, 20 };
        }
        if (width <= 85) {
            return { width, 80 };
        }
        return { width, 40 };
    }

    AMinMaxAxis onComputeIntrinsicMinMaxAxis(int) override {
        return {
            .min = 0,
            .max = 200,
        };
    }

    [[nodiscard]]
    int lastMeasuredWidth() const noexcept {
        return mLastMeasuredWidth;
    }

private:
    int mLastMeasuredWidth = -1;
};

class MinimumWidthView : public AView {
public:
    AMinMaxAxis onComputeIntrinsicMinMaxAxis(int) override {
        return {
            .min = 40,
            .max = 40,
        };
    }
};

class FixedHeightOverflowingWidthView : public AView {
public:
    glm::ivec2 onIntrinsicMeasure(AConstraints constraints) override {
        const int width = constraints.isUnlimitedInline() ? 150 : constraints.maxInline;
        return { width, 20 };
    }

    AMinMaxAxis onComputeIntrinsicMinMaxAxis(int) override {
        return {
            .min = 150,
            .max = 150,
        };
    }
};

class FlexibleWideView : public AView {
public:
    glm::ivec2 onIntrinsicMeasure(AConstraints constraints) override {
        const int width = constraints.isUnlimitedInline() ? 6000 : constraints.maxInline;
        mLastMeasuredWidth = width;
        return { width, 20 };
    }

    AMinMaxAxis onComputeIntrinsicMinMaxAxis(int) override {
        return {
            .min = 0,
            .max = 6000,
        };
    }

    [[nodiscard]]
    int lastMeasuredWidth() const noexcept {
        return mLastMeasuredWidth;
    }

private:
    int mLastMeasuredWidth = -1;
};

class FixedContentView : public AView {
public:
    explicit FixedContentView(glm::ivec2 size) : mSize(size) {}

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

TEST(AScrollArea, MeasureGrowsToAvoidVerticalScrollbarWhenHeightIsUnbounded) {
    auto content = _new<TrackingWrappingView>();

    AScrollArea scrollArea;
    scrollArea.setContents(content);

    const auto measured = scrollArea.measure({
        .maxInline = 100,
        .maxBlock = -1,
    });

    EXPECT_EQ(measured, glm::ivec2(100, 40));
}

TEST(AScrollArea, MeasureWithUnboundedConstraintsReturnsNaturalContentSize) {
    auto content = _new<TrackingWrappingView>();

    AScrollArea scrollArea;
    scrollArea.setContents(content);

    const auto measured = scrollArea.measure({});

    EXPECT_EQ(measured, glm::ivec2(200, 20));
}

TEST(AScrollArea, MeasureWithZeroWidthUsesWrappedContentHeight) {
    auto content = _new<TrackingWrappingView>();

    AScrollArea scrollArea;
    scrollArea.setContents(content);

    const auto measured = scrollArea.measure(AConstraints::fixedInline(0));

    EXPECT_EQ(measured, glm::ivec2(0, 80));
}

TEST(AScrollArea, MeasureKeepsBoundedOuterHeightWhenHorizontalOverflowNeedsScrollbars) {
    auto content = _new<FixedHeightOverflowingWidthView>();

    AScrollArea scrollArea;
    scrollArea.setContents(content);

    const auto measured = scrollArea.measure({
        .maxInline = 100,
        .maxBlock = -1,
    });

    EXPECT_EQ(measured, glm::ivec2(100, 20));
}

TEST(AScrollArea, MeasureWithZeroSizedViewportProbeReturnsMinimumChromeSizePlusPadding) {
    auto content = _new<TrackingWrappingView>();

    AScrollArea scrollArea;
    scrollArea.setPadding({ .left = 3, .right = 5, .top = 7, .bottom = 11 });
    scrollArea.setContents(content);

    const int expectedWidth =
        scrollArea.verticalScrollbar()->measure(AConstraints {}).x * 2 + scrollArea.getPadding().horizontal();
    const int expectedHeight =
        scrollArea.horizontalScrollbar()->measure(AConstraints {}).y * 2 + scrollArea.getPadding().vertical();

    const auto measured = scrollArea.measure({
        .maxInline = 0,
        .maxBlock = 0,
    });

    EXPECT_EQ(measured, glm::ivec2(expectedWidth, expectedHeight));
}

TEST(AScrollArea, MeasureCapsBoundedWidthToNaturalContentWidth) {
    auto content = _new<FixedContentView>(glm::ivec2(200, 200));

    AScrollArea scrollArea;
    scrollArea.setContents(content);

    EXPECT_EQ(scrollArea.measure({
                  .maxInline = 40,
                  .maxBlock = -1,
              }).x,
              40);
    EXPECT_EQ(scrollArea.measure({
                  .maxInline = 80,
                  .maxBlock = -1,
              }).x,
              80);
    EXPECT_EQ(scrollArea.measure({
                  .maxInline = 300,
                  .maxBlock = -1,
              }).x,
              200);
}

TEST(AScrollArea, LayoutPrefersViewportWidthWhenContentCanShrinkWithoutOverflow) {
    auto content = _new<FlexibleWideView>();

    AScrollArea scrollArea;
    scrollArea.setContents(content);
    scrollArea.layout({ 0, 0 }, { 100, 40 });

    EXPECT_FALSE(bool(scrollArea.horizontalScrollbar()->getVisibility() & Visibility::FLAG_RENDER_NEEDED));
    EXPECT_EQ(content->lastMeasuredWidth(), 100);
    EXPECT_EQ(content->getWidth(), 100);
}

TEST(AScrollAreaViewport, MeasureUsesContentSize) {
    auto content = _new<TrackingWrappingView>();

    AScrollAreaViewport viewport;
    viewport.setContents(content);

    const auto measured = viewport.measure({
        .maxInline = 100,
        .maxBlock = -1,
    });

    EXPECT_EQ(measured, glm::ivec2(100, 40));
    EXPECT_EQ(content->lastMeasuredWidth(), 100);
}

TEST(AScrollAreaViewport, MeasureWithZeroWidthReturnsZeroViewportSize) {
    auto content = _new<TrackingWrappingView>();

    AScrollAreaViewport viewport;
    viewport.setContents(content);

    const auto measured = viewport.measure(AConstraints::fixedInline(0));

    EXPECT_EQ(measured, glm::ivec2(0, 0));
}

TEST(AScrollArea, VerticalOverflowReducesViewportWidthWithoutHorizontalScroll) {
    auto content = _new<TrackingWrappingView>();

    AScrollArea scrollArea;
    scrollArea.setContents(content);
    scrollArea.layout({ 0, 0}, { 100, 30 });

    EXPECT_TRUE(bool(scrollArea.verticalScrollbar()->getVisibility() & Visibility::FLAG_RENDER_NEEDED));
    EXPECT_FALSE(bool(scrollArea.horizontalScrollbar()->getVisibility() & Visibility::FLAG_RENDER_NEEDED));

    const int viewportWidth = scrollArea.getWidth() - scrollArea.verticalScrollbar()->getWidth();
    EXPECT_EQ(content->lastMeasuredWidth(), viewportWidth);
    EXPECT_EQ(scrollArea.verticalScrollbar()->getPosition().x, viewportWidth);
}

TEST(AScrollArea, MinimumWidthContentUsesHorizontalScrollbar) {
    auto content = _new<MinimumWidthView>();
    content->setCustomStyle({
        ass::MinSize { AMetric(150, AMetric::T_PX), AMetric(20, AMetric::T_PX) }
    });

    AScrollArea scrollArea;
    scrollArea.setContents(content);
    scrollArea.layout({ 0, 0}, { 100, 40 });

    EXPECT_TRUE(static_cast<bool>(scrollArea.horizontalScrollbar()->getVisibility() & Visibility::FLAG_RENDER_NEEDED));
    EXPECT_FALSE(static_cast<bool>(scrollArea.verticalScrollbar()->getVisibility() & Visibility::FLAG_RENDER_NEEDED));
    EXPECT_EQ(scrollArea.horizontalScrollbar()->getPosition().y,
              scrollArea.getHeight() - scrollArea.horizontalScrollbar()->getHeight());
    EXPECT_EQ(scrollArea.horizontalScrollbar()->getWidth(), 100);
}

TEST(AScrollArea, MaxSizeIsLargerThanMinSize) {
  class SmallView : public AView {
  public:
    SmallView() {
      setFixedSize({200, 200});
    }
  };

  auto scrollArea = _new<AScrollArea>();
  scrollArea->setContents(_new<SmallView>());

  auto minMax = scrollArea->computeMinMaxAxis();

  EXPECT_GT(minMax.max, minMax.min);
}
