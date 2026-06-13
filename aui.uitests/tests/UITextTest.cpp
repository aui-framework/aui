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

#include <AUI/UITest.h>
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/AText.h>
#include <AUI/View/AScrollArea.h>
#include <gmock/gmock.h>
#include <algorithm>

using namespace ass;
using namespace declarative;


class UIText: public testing::UITest {
public:
    ~UIText() override = default;

};


TEST_F(UIText, Contents) {
    auto window = _new<AWindow>();
    auto text = AText::fromString("Привет мир\nвторая строка");
    window->setContents(Centered {
        text
    });
    window->pack();
    window->show();
    auto& entries = text->mEngine.entries();
    ASSERT_EQ(entries.size(), 7);
    EXPECT_EQ(AString::fromUtf32(_cast<AText::WordEntry>(entries[0])->getWord()), "Привет");
    EXPECT_NE(_cast<aui::detail::WhitespaceEntry>(entries[1]), nullptr);
    EXPECT_EQ(AString::fromUtf32(_cast<AText::WordEntry>(entries[2])->getWord()), "мир");
    EXPECT_NE(_cast<aui::detail::NextLineEntry>(entries[3]), nullptr);
    EXPECT_EQ(AString::fromUtf32(_cast<aui::detail::WordEntry>(entries[4])->getWord()), "вторая");
    EXPECT_NE(_cast<aui::detail::WhitespaceEntry>(entries[5]), nullptr);
    EXPECT_EQ(AString::fromUtf32(_cast<aui::detail::WordEntry>(entries[6])->getWord()), "строка");
}

namespace {

class ViewMock: public ALabel {
public:
    ViewMock(): ALabel("Test") {}
};
}

TEST_F(UIText, Basic) {
    auto w = _new<AWindow>();
    auto mock = _new<ViewMock>();
    auto text = AText::fromItems({
        "Hello", mock, "World", "Test"
    }) AUI_OVERRIDE_STYLE {
        FixedSize { 200_dp }
    };

    w->setContents(Centered {
        text,
    });
    w->pack();
    w->show();
    uitest::frame();

    const auto position = mock->getPosition();
    const int helloWidth = text->getFontStyle().getWidth(U"Hello");
    EXPECT_THAT(position.x, testing::AllOf(testing::Ge(helloWidth - 1), testing::Le(helloWidth + 1)));
    EXPECT_THAT(position.y, testing::AllOf(testing::Ge(0), testing::Le(5)));
}

TEST_F(UIText, Visibility) {
    auto w = _new<AWindow>();
    _<AView> label = Label { "Hello" };
    auto mock = _new<ViewMock>();
    auto text = AText::fromItems({
        label, mock,
    }) AUI_OVERRIDE_STYLE {
        FixedSize { 200_dp }
    };
    const int labelWidth = label->computeMinMaxAxis().max + label->getMargin().horizontal();

    testing::InSequence s;

    w->setContents(Centered {
        text,
    });
    w->pack();
    w->show();
    uitest::frame();

    EXPECT_THAT(mock->getPosition().x, testing::AllOf(testing::Ge(labelWidth - 1), testing::Le(labelWidth + 1)));
    EXPECT_THAT(mock->getPosition().y, testing::AllOf(testing::Ge(0), testing::Le(5)));

    saveScreenshot("initial");

    label->setVisibility(Visibility::GONE);
    uitest::frame();

    EXPECT_THAT(mock->getPosition().x, testing::AllOf(testing::Ge(0), testing::Le(5)));
    EXPECT_THAT(mock->getPosition().y, testing::AllOf(testing::Ge(0), testing::Le(5)));

    saveScreenshot("label gone");

    label->setVisibility(Visibility::VISIBLE);
    uitest::frame();

    EXPECT_THAT(mock->getPosition().x, testing::AllOf(testing::Ge(labelWidth - 1), testing::Le(labelWidth + 1)));
    EXPECT_THAT(mock->getPosition().y, testing::AllOf(testing::Ge(0), testing::Le(5)));

    saveScreenshot("label visible");
}

TEST_F(UIText, MeasureUsesTightWidthForWrapping) {
    auto text = AText::fromString("alpha beta gamma delta epsilon zeta eta theta");

    const auto unconstrained = text->measure(AConstraints {});
    ASSERT_GT(unconstrained.x, 0);
    ASSERT_GT(unconstrained.y, 0);

    const int tightWidth = std::max(1, unconstrained.x / 2);
    const auto constrained = text->measure({
        .maxInline = tightWidth,
    });

    EXPECT_EQ(constrained.x, tightWidth);
    EXPECT_GT(constrained.y, unconstrained.y);
}

TEST_F(UIText, MeasureHeightGrowsWhenWidthShrinks) {
    auto text = AText::fromString("one two three four five six seven eight nine ten");

    const auto wide = text->measure({
        .maxInline = 220,
    });
    const auto narrow = text->measure({
        .maxInline = 120,
    });

    EXPECT_EQ(wide.x, 220);
    EXPECT_EQ(narrow.x, 120);
    EXPECT_GE(narrow.y, wide.y);
}

TEST_F(UIText, MeasureIncludesPadding) {
    auto plain = AText::fromString("padding probe");
    auto padded = AText::fromString("padding probe");
    padded->setPadding({ .left = 7, .right = 11, .top = 3, .bottom = 5 });

    const auto plainMeasured = plain->measure(AConstraints {});
    const auto paddedMeasured = padded->measure(AConstraints {});

    EXPECT_EQ(paddedMeasured.x, plainMeasured.x + 18);
    EXPECT_EQ(paddedMeasured.y, plainMeasured.y + 8);
}

TEST_F(UIText, BreakAllAllowsNarrowIntrinsicWidthAndWrapping) {
    auto text = AText::fromString("supercalifragilisticexpialidocious", { WordBreak::BREAK_ALL });

    const auto unconstrained = text->measure(AConstraints {});
    const auto constrained = text->measure({
        .maxInline = std::max(1, unconstrained.x / 2),
    });

    EXPECT_LT(text->computeMinMaxAxis().min, unconstrained.x);
    EXPECT_EQ(constrained.x, std::max(1, unconstrained.x / 2));
    EXPECT_GT(constrained.y, unconstrained.y);
}
