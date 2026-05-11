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
#include <AUI/View/AView.h>
#include <functional>

template<typename T>
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

    AStackedLayout layout;
    layout.addView(visible, std::nullopt);
    layout.addView(hidden, std::nullopt);
    layout.layout(0, 0, 100, 100);

    expectRect(visible, { 40, 45 }, { 20, 10 });
    EXPECT_EQ(hidden->getSize(), glm::ivec2());
}
