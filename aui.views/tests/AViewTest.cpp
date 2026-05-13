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
#include <AUI/Layout/AHorizontalLayout.h>
#include <AUI/View/AAbstractLabel.h>
#include <AUI/View/AView.h>
#include <AUI/View/AViewContainer.h>
#include <functional>
#include <limits>

template <typename T>
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

class MeasureTestView : public AView {
public:
  int intrinsicWidth = 0;
  int intrinsicHeight = 0;
  glm::ivec2 measuredContentSize = {};
  int lastWidthConstraint = std::numeric_limits<int>::min();
  int lastHeightConstraint = std::numeric_limits<int>::min();
  AConstraints lastMeasureConstraints {};

  glm::ivec2 onIntrinsicMeasure(AConstraints constraints) override {
    lastMeasureConstraints = constraints;
    if (constraints.isInlineTight()) {
      lastWidthConstraint = constraints.maxInline;
    }
    if (constraints.isBlockTight()) {
      lastHeightConstraint = constraints.maxBlock;
    }
    return measuredContentSize;
  }

  AMinMaxAxis onComputeIntrinsicMinMaxAxis(int height) override {
    lastHeightConstraint = height;
    return {
      .min = {},
      .max = intrinsicWidth,
    };
  }
};

class MinMaxTestView : public AView {
public:
  int calls = 0;
  AMinMaxAxis value;

  AMinMaxAxis onComputeIntrinsicMinMaxAxis(int) override {
    ++calls;
    return value;
  }
};

class LabelMinMaxTestView : public AAbstractLabel {
public:
  AMinMaxAxis onComputeIntrinsicMinMaxAxis(int) override {
    return {
      .min = 17,
      .max = 17,
    };
  }
};

}   // namespace

TEST(AView, MeasureUsesPaddingAdjustedConstraints) {
  MeasureTestView view;
  view.setPadding({ .left = 3, .right = 5, .top = 7, .bottom = 11 });
  view.intrinsicWidth = 10;
  view.intrinsicHeight = 9;

  EXPECT_EQ(view.measure(AConstraints::fixedBlock(40)).x, 18);
  EXPECT_EQ(view.lastHeightConstraint, 22);

  EXPECT_EQ(view.measure(AConstraints::fixedInline(50)).y, 27);
  EXPECT_EQ(view.lastWidthConstraint, 42);
}

TEST(AView, MeasurePassesContentConstraintsAndAddsPaddingBack) {
  MeasureTestView view;
  view.setPadding({ .left = 3, .right = 5, .top = 7, .bottom = 11 });
  view.measuredContentSize = { 20, 10 };

  auto measured = view.measure({
    .minInline = 30,
    .maxInline = 40,
    .minBlock = 25,
    .maxBlock = 60,
  });

  EXPECT_EQ(view.lastMeasureConstraints.minInline, 22);
  EXPECT_EQ(view.lastMeasureConstraints.maxInline, 32);
  EXPECT_EQ(view.lastMeasureConstraints.minBlock, 7);
  EXPECT_EQ(view.lastMeasureConstraints.maxBlock, 42);
  EXPECT_EQ(measured, glm::ivec2(30, 28));
}

TEST(AView, MeasureHonorsMinAndMaxSize) {
  MeasureTestView view;
  view.setMinSize({ 20, 30 });
  view.setMaxSize({ 40, 50 });
  view.measuredContentSize = { 10, 60 };

  auto measured = view.measure(AConstraints {});

  EXPECT_EQ(view.lastMeasureConstraints.minInline, 20);
  EXPECT_EQ(view.lastMeasureConstraints.maxInline, 40);
  EXPECT_EQ(view.lastMeasureConstraints.minBlock, 30);
  EXPECT_EQ(view.lastMeasureConstraints.maxBlock, 50);
  EXPECT_EQ(measured, glm::ivec2(20, 50));
}

TEST(AView, MeasureHonorsFixedSizeAndContentConstraints) {
  MeasureTestView view;
  view.setPadding({ .left = 4, .right = 6, .top = 2, .bottom = 8 });
  view.setFixedSize({ 50, 30 });
  view.measuredContentSize = { 0, 0 };

  auto measured = view.measure(AConstraints {});

  EXPECT_EQ(view.lastMeasureConstraints.minInline, 40);
  EXPECT_EQ(view.lastMeasureConstraints.maxInline, 40);
  EXPECT_EQ(view.lastMeasureConstraints.minBlock, 20);
  EXPECT_EQ(view.lastMeasureConstraints.maxBlock, 20);
  EXPECT_EQ(measured, glm::ivec2(50, 30));
}

TEST(AView, ComputeMinMaxSizesUsesIntrinsicSemantics) {
  MeasureTestView view;
  view.setMinSize({ 20, 30 });
  view.setPadding({ .left = 3, .right = 5, .top = 7, .bottom = 11 });
  view.measuredContentSize = { 10, 5 };
  view.intrinsicWidth = 10;
  view.intrinsicHeight = 9;

  const auto minMax = view.computeMinMaxAxis();

  EXPECT_EQ(minMax.min, 20);
  EXPECT_EQ(minMax.max, 20);
}

TEST(AView, ComputeMinMaxSizesIsCachedUntilRequestLayout) {
  MinMaxTestView view;
  view.value = {
    .min = 1,
    .max = 3,
  };

  EXPECT_EQ(view.computeMinMaxAxis().max, 3);
  EXPECT_EQ(view.computeMinMaxAxis().max, 3);
  EXPECT_EQ(view.calls, 1);

  view.value = {
    .min = 5,
    .max = 7,
  };
  view.requestLayout();

  EXPECT_EQ(view.computeMinMaxAxis().max, 7);
  EXPECT_EQ(view.calls, 2);
}

TEST(AView, ViewContainerForwardsLayoutMinMaxSizes) {
  auto left = _new<FakeLayoutItem>();
  left->setMinSize({ 10, 5 });
  left->preferredWidth = [](int) { return 30; };
  left->preferredHeight = [](int) { return 20; };

  auto right = _new<FakeLayoutItem>();
  right->setMinSize({ 7, 11 });
  right->preferredWidth = [](int) { return 40; };
  right->preferredHeight = [](int) { return 13; };

  AViewContainer container;
  container.setLayout(std::make_unique<AHorizontalLayout>());
  container.addView(left);
  container.addView(right);

  const auto minMax = container.computeMinMaxAxis();

  EXPECT_EQ(minMax.min, 17);
  EXPECT_EQ(minMax.max, 70);
}

TEST(AView, AbstractLabelUsesExactIntrinsicMinMaxSizes) {
  LabelMinMaxTestView view;

  const auto minMax = view.computeMinMaxAxis();

  EXPECT_EQ(minMax.min, 17);
  EXPECT_EQ(minMax.max, 17);
}
