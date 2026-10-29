#include <gtest/gtest.h>

#include "ui/view/ViewportMapping.h"

using aui_remote::ViewportMapping;

TEST(ViewportMapping, IdentityWhenSizesMatch) {
    ViewportMapping m { { 800, 600 }, { 800, 600 } };
    EXPECT_FLOAT_EQ(m.scale(), 1.f);
    ASSERT_TRUE(m.toWindow({ 100, 50 }));
    EXPECT_EQ(*m.toWindow({ 100, 50 }), glm::vec2(100, 50));
}

TEST(ViewportMapping, ScalesUniformly) {
    ViewportMapping m { { 400, 300 }, { 800, 600 } };
    EXPECT_FLOAT_EQ(m.scale(), 0.5f);
    EXPECT_EQ(*m.toWindow({ 200, 150 }), glm::vec2(400, 300));
}

TEST(ViewportMapping, LetterboxesWideViewport) {
    // window 400x200 shown in 1000x200: content is 400x200 centered, bars 300px on both sides
    ViewportMapping m { { 1000, 200 }, { 400, 200 } };
    EXPECT_FALSE(m.toWindow({ 100, 100 }));
    EXPECT_FALSE(m.toWindow({ 900, 100 }));
    ASSERT_TRUE(m.toWindow({ 300, 0 }));
    EXPECT_EQ(*m.toWindow({ 300, 0 }), glm::vec2(0, 0));
    EXPECT_EQ(*m.toWindow({ 500, 100 }), glm::vec2(200, 100));
}

TEST(ViewportMapping, LetterboxesTallViewport) {
    ViewportMapping m { { 200, 400 }, { 100, 100 } };
    EXPECT_FLOAT_EQ(m.scale(), 2.f);
    EXPECT_FALSE(m.toWindow({ 100, 50 }));
    EXPECT_EQ(*m.toWindow({ 100, 200 }), glm::vec2(50, 50));
}

TEST(ViewportMapping, ClampedKeepsDragInsideWindow) {
    ViewportMapping m { { 1000, 200 }, { 400, 200 } };
    EXPECT_EQ(m.toWindowClamped({ -50, 100 }), glm::vec2(0, 100));
    EXPECT_EQ(m.toWindowClamped({ 5000, 900 }), glm::vec2(399, 199));
}

TEST(ViewportMapping, InvalidSizes) {
    ViewportMapping m { { 0, 0 }, { 400, 200 } };
    EXPECT_FALSE(m.toWindow({ 1, 1 }));
    EXPECT_EQ(m.toWindowClamped({ 1, 1 }), glm::vec2(0, 0));
}

TEST(ViewportMapping, ToViewportIsInverseOfToWindow) {
    ViewportMapping m { { 1000, 200 }, { 800, 200 } };   // scale 1, bars of 100px
    EXPECT_EQ(m.toViewport({ 0, 0 }), glm::vec2(100, 0));
    ViewportMapping half { { 400, 300 }, { 800, 600 } };
    EXPECT_EQ(half.toViewport({ 400, 300 }), glm::vec2(200, 150));
    EXPECT_EQ(*half.toWindow(half.toViewport({ 100, 50 })), glm::vec2(100, 50));
}
