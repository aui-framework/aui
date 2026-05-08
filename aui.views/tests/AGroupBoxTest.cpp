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
#include <AUI/Layout/HVLayout.h>
#include <AUI/Layout/AHorizontalLayout.h>
#include <AUI/Layout/AVerticalLayout.h>
#include <AUI/View/AGroupBox.h>
#include <AUI/View/AView.h>
#include <AUI/View/AViewContainer.h>

namespace {

using VerticalHVLayout = aui::HVLayout<ALayoutDirection::VERTICAL>;

class FixedMeasureView : public AView {
public:
    FixedMeasureView(int width, int height) : mWidth(width), mHeight(height) {}

    int onComputeIntrinsicWidth(int height) override {
        return mWidth;
    }

    int onComputeIntrinsicHeight(int width) override {
        return mHeight;
    }

private:
    int mWidth;
    int mHeight;
};

class PreferredButMinConstrainedView : public AView {
public:
    PreferredButMinConstrainedView(int preferredWidth, int preferredHeight, glm::ivec2 minSize)
        : mPreferredWidth(preferredWidth), mPreferredHeight(preferredHeight) {
      setCustomStyle({
        ass::MinSize { AMetric(minSize.x, AMetric::T_PX), AMetric(minSize.y, AMetric::T_PX) }
      });
    }

    int onComputeIntrinsicWidth(int height) override {
        return mPreferredWidth;
    }

    int onComputeIntrinsicHeight(int width) override {
        return mPreferredHeight;
    }

private:
    int mPreferredWidth;
    int mPreferredHeight;
};

template<typename T>
void expectRect(const _<T>& item, glm::ivec2 position, glm::ivec2 size) {
    EXPECT_EQ(item->getPosition(), position);
    EXPECT_EQ(item->getSize(), size);
}

}   // namespace

TEST(HVLayout, VerticalLayoutKeepsChildPerpendicularMinimumSize) {
    auto child = _new<PreferredButMinConstrainedView>(120, 20, glm::ivec2(60, 0));

    AVector<_<AView>> views { child };
    VerticalHVLayout::layout({ 0, 0 }, { 40, 100 }, views, 0);

    expectRect(child, { 0, 0 }, { 60, 20 });
}

TEST(AGroupBox, KeepsMinimumWidthOfContentWhenParentIsNarrower) {
    auto groupBox = _new<AGroupBox>(
        _new<FixedMeasureView>(20, 10),
        _new<PreferredButMinConstrainedView>(120, 20, glm::ivec2(60, 0)));

    auto parent = _new<AViewContainer>();
    parent->setLayout(std::make_unique<AVerticalLayout>());
    parent->addView(groupBox);

    parent->layout(0, 0, 60, 100);

    EXPECT_EQ(groupBox->getWidth(), 140);
}

TEST(AGroupBox, KeepsMinimumWidthThroughExpandingParentLayout) {
    auto groupBox = _new<AGroupBox>(
        _new<FixedMeasureView>(20, 10),
        _new<PreferredButMinConstrainedView>(120, 20, glm::ivec2(60, 0)));
    groupBox->setExpanding({ 1, 0 });

    auto parent = _new<AViewContainer>();
    parent->setLayout(std::make_unique<AHorizontalLayout>());
    parent->addView(groupBox);

    parent->layout(0, 0, 40, 100);

    EXPECT_EQ(groupBox->getWidth(), 140);
}
