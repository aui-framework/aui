#include <gtest/gtest.h>

#include "Cli/ImageCompare.h"
#include <AUI/Image/AImage.h>

using namespace aui_remote::cli;

namespace {
AImage solid(glm::uvec2 size, AColor color) {
    AImage image(size, APixelFormat::RGBA_BYTE);
    image.fill(color);
    return image;
}
}   // namespace

TEST(ImageCompareTest, Identical) {
    auto diff = compareImages(solid({ 4, 4 }, AColor::RED), solid({ 4, 4 }, AColor::RED));
    EXPECT_EQ(diff.differentPixels, 0u);
    EXPECT_EQ(diff.percent(), 0.0);
}

TEST(ImageCompareTest, CountsDifferentPixels) {
    auto a = solid({ 10, 10 }, AColor::BLACK);
    auto b = solid({ 10, 10 }, AColor::BLACK);
    for (unsigned x = 0; x < 10; ++x) {
        b.set({ x, 0 }, AColor::WHITE);
    }
    auto diff = compareImages(a, b);
    EXPECT_EQ(diff.differentPixels, 10u);
    EXPECT_DOUBLE_EQ(diff.percent(), 10.0);
}

TEST(ImageCompareTest, ThresholdIgnoresSmallDifferences) {
    auto a = solid({ 2, 2 }, AColor(0.5f, 0.5f, 0.5f, 1.f));
    auto b = solid({ 2, 2 }, AColor(0.52f, 0.5f, 0.5f, 1.f));
    EXPECT_EQ(compareImages(a, b, 0).differentPixels, 4u);
    EXPECT_EQ(compareImages(a, b, 10).differentPixels, 0u);
}

TEST(ImageCompareTest, SizeMismatchIsFullyDifferent) {
    auto diff = compareImages(solid({ 2, 2 }, AColor::RED), solid({ 3, 2 }, AColor::RED));
    EXPECT_TRUE(diff.sizeMismatch);
    EXPECT_EQ(diff.percent(), 100.0);
}
