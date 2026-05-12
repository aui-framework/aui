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

}   // namespace

TEST(AScrollArea, MeasurePrefersAvailableWidthForWrappingContent) {
    auto content = _new<TrackingWrappingView>();

    AScrollArea scrollArea;
    scrollArea.setContents(content);

    const auto measured = scrollArea.measure({
        .maxInline = 100,
        .maxBlock = -1,
    });

    EXPECT_EQ(measured, glm::ivec2(100, 40));
    EXPECT_EQ(content->lastMeasuredWidth(), 100);
}

TEST(AScrollArea, MeasureWithUnboundedWidthReturnsMinimumOnly) {
    auto content = _new<TrackingWrappingView>();

    AScrollArea scrollArea;
    scrollArea.setContents(content);

    const auto measured = scrollArea.measure({});

    EXPECT_EQ(measured, glm::ivec2(0, 0));
}

TEST(AScrollArea, MeasureWithZeroWidthUsesNaturalContentHeightPlusScrollbar) {
    auto content = _new<TrackingWrappingView>();

    AScrollArea scrollArea;
    scrollArea.setContents(content);

    const int horizontalScrollbarMinWidth = scrollArea.horizontalScrollbar()->computeMinMaxAxis().min;
    const int verticalScrollbarMinHeight = scrollArea.verticalScrollbar()->measure(AConstraints {}).y;
    const auto measured = scrollArea.measure(AConstraints::fixedInline(0));

    EXPECT_EQ(measured, glm::ivec2(horizontalScrollbarMinWidth, verticalScrollbarMinHeight));
}

TEST(AScrollArea, MeasureAddsHorizontalScrollbarHeightForWidthOverflow) {
    auto content = _new<FixedHeightOverflowingWidthView>();

    AScrollArea scrollArea;
    scrollArea.setContents(content);

    const int scrollbarHeight = scrollArea.horizontalScrollbar()->measure(AConstraints::fixedInline(100)).y;
    const auto measured = scrollArea.measure({
        .maxInline = 100,
        .maxBlock = -1,
    });

    EXPECT_EQ(measured, glm::ivec2(100, 20 + scrollbarHeight));
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

TEST(AScrollbar, MeasureIsStableForZeroMainAxisConstraint) {
    AScrollbar horizontal(ALayoutDirection::HORIZONTAL);
    AScrollbar vertical(ALayoutDirection::VERTICAL);

    const auto horizontalMeasured = horizontal.measure(AConstraints::fixedInline(0));
    const auto verticalMeasured = vertical.measure(AConstraints::fixedBlock(0));

    EXPECT_EQ(horizontalMeasured.x, 0);
    EXPECT_GT(horizontalMeasured.y, 0);
    EXPECT_EQ(verticalMeasured.y, 0);
    EXPECT_GT(verticalMeasured.x, 0);
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
