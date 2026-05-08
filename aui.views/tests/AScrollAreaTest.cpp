#include <gtest/gtest.h>
#include <AUI/ASS/ASS.h>
#include <AUI/Util/AMetric.h>
#include <AUI/View/AScrollArea.h>
#include <AUI/View/AScrollAreaViewport.h>

namespace {

class TrackingWrappingView : public AView {
public:
    int onComputeIntrinsicWidth(int height) override {
        return 200;
    }

    int onComputeIntrinsicHeight(int width) override {
        mLastMeasuredWidth = width;
        if (width == -1 || width > 100) {
            return 20;
        }
        if (width <= 85) {
            return 80;
        }
        return 40;
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
    int onComputeIntrinsicWidth(int height) override {
        return 40;
    }

    int onComputeIntrinsicHeight(int width) override {
        return 20;
    }
};

}   // namespace

TEST(AScrollArea, MeasurePrefersAvailableWidthForWrappingContent) {
    auto content = _new<TrackingWrappingView>();

    AScrollArea scrollArea;
    scrollArea.setContents(content);

    const auto measured = scrollArea.measure({
        .maxWidth = 100,
        .maxHeight = 1000000,
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
        .maxHeight = 1000000,
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
