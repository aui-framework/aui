#include <gtest/gtest.h>
#include <AUI/ASS/ASS.h>
#include <AUI/Util/AMetric.h>
#include <AUI/View/AScrollArea.h>
#include <AUI/View/AScrollAreaViewport.h>

namespace {

class TrackingWrappingView : public AView {
public:
    glm::ivec2 onIntrinsicMeasure(AConstraints constraints) override {
        const int width = constraints.isUnlimitedWidth() ? 200 : constraints.maxWidth;
        mLastMeasuredWidth = width;
        if (width > 100) {
            return { width, 20 };
        }
        if (width <= 85) {
            return { width, 80 };
        }
        return { width, 40 };
    }

    AMinMaxSizes onComputeIntrinsicMinMaxSizes(int) override {
        return {
            .min = { 200, 20 },
            .max = { 200, 20 },
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
    AMinMaxSizes onComputeIntrinsicMinMaxSizes(int) override {
        return {
            .min = { 40, 20 },
            .max = { 40, 20 },
        };
    }
};

}   // namespace

TEST(AScrollArea, MeasurePrefersAvailableWidthForWrappingContent) {
    auto content = _new<TrackingWrappingView>();

    AScrollArea scrollArea;
    scrollArea.setContents(content);

    const auto measured = scrollArea.measure({
        .maxWidth = 100,
        .maxHeight = -1,
    });

    EXPECT_EQ(measured, glm::ivec2(100, 40));
    EXPECT_EQ(content->lastMeasuredWidth(), 100);
}

TEST(AScrollAreaViewport, MeasureUsesContentSize) {
    auto content = _new<TrackingWrappingView>();

    AScrollAreaViewport viewport;
    viewport.setContents(content);

    const auto measured = viewport.measure({
        .maxWidth = 100,
        .maxHeight = -1,
    });

    EXPECT_EQ(measured, glm::ivec2(100, 40));
    EXPECT_EQ(content->lastMeasuredWidth(), 100);
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
