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
#include <AUI/Layout/AStackedLayout.h>
#include <AUI/View/AViewContainer.h>
#include <AUI/View/AView.h>
#include <AUI/View/ALabel.h>
#include <AUI/Util/Declarative/Containers.h>
#include <functional>

template<typename T>
inline std::ostream& operator<<(std::ostream& o, const glm::tvec2<T>& v) {
    return o << "{ " << v.x << ", " << v.y << " }";
}

using namespace declarative;

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

template<typename T>
void expectRect(const _<T>& item, glm::ivec2 position, glm::ivec2 size) {
    EXPECT_EQ(item->getPosition(), position);
    EXPECT_EQ(item->getSize(), size);
}

}   // namespace

TEST(AStackedLayout, IgnoresHiddenViewsInLayout) {
  auto visible = _new<FakeLayoutItem>();
  visible->preferredWidth = [](int) { return 20; };
  visible->preferredHeight = [](int) { return 10; };

  auto hidden = _new<FakeLayoutItem>();
  hidden->preferredWidth = [](int) { return 100; };
  hidden->preferredHeight = [](int) { return 50; };
  hidden->setVisibility(Visibility::GONE);
  hidden->setSize(glm::ivec2());

  AStackedLayout layout;
  layout.addView(visible, std::nullopt);
  layout.addView(hidden, std::nullopt);
  layout.layout(0, 0, 100, 100);

  expectRect(visible, { 40, 45 }, { 20, 10 });
  EXPECT_EQ(hidden->getSize(), glm::ivec2());
}

TEST(AStackedLayout, ConsistencyTest) {
  _<AView> container = Stacked {
    Label { "Some Long Label" }
  } AUI_OVERRIDE_STYLE {
    MinSize { 60_px, 22_px },
    Padding { 12_px, 4_px },
  };

  auto minMax = container->computeMinMaxAxis();
  int minWidth = minMax.min;

  // Measure with maxInline = -1
  auto sizeUnlimited = container->measure(AConstraints { .minInline = 0, .maxInline = -1 });
  EXPECT_GE(sizeUnlimited.x, minWidth);

  // Measure with maxInline = 0
  // According to AView::measure, if minInline=0 and maxInline=0, it should use computeMinMaxAxis().min
  auto sizeZeroMax = container->measure(AConstraints { .minInline = 0, .maxInline = 0 });

  EXPECT_EQ(sizeZeroMax.x, minWidth) << "Width with maxInline=0 should be equal to minWidth";
}
