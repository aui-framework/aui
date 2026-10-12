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
#include <AUI/View/ATextField.h>
#include "InkGaps.h"

using namespace ass;
using namespace declarative;
using std::operator""sv;

namespace {
    struct SelectionMatchesAssert {
        ACursorSelectable::Selection selection;

        SelectionMatchesAssert(unsigned begin, unsigned end) : selection{begin, end} {}

        SelectionMatchesAssert(unsigned s) : selection{s, s} {}

        bool operator()(const _<AView>& view) const {
            if (auto t = _cast<ACursorSelectable>(view)) {
                EXPECT_EQ(t->selection(), selection);
            }
            return true;
        }
    };

    using selectionMatches = SelectionMatchesAssert;
}

class UITextField: public testing::UITest {
public:
    ~UITextField() override = default;

protected:
    void SetUp() override {
        UITest::SetUp();

        class TestWindow: public AWindow {
        public:
            TestWindow() {
                setContents(Centered {
                        _new<ATextField>() AUI_OVERRIDE_STYLE { FixedSize { 300_dp, {} } } AUI_LET {
                            it->setText("hello world!");
                        }
                });
                pack();
            }
        };

        _new<TestWindow>()->show();
        uitest::frame();
    }

    void TearDown() override {
        UITest::TearDown();
    }
};

/**
 * Checks that then clicking at the left border of the text field, cursor jumps to the first symbol.
 */
TEST_F(UITextField, CursorAppearsAtTheStart) {
    By::type<ATextField>().perform(click({0_dp, 0_dp})) // click at the left border
                          .check(selectionMatches(0));
}

/**
 * Checks that then clicking at the right border of the text field, cursor jumps to the last symbol.
 */
TEST_F(UITextField, CursorAppearsAtTheEnd) {
    By::type<ATextField>().perform(click({299_dp, 0_dp})) // click at the right border
                          .check(selectionMatches(12));
}

/**
 * Checks that when doubleclicking a whole word is selected
 */
TEST_F(UITextField, DoubleClickWordSelection1) {
    By::type<ATextField>().perform(doubleClick({20_dp, 0_dp})) // click at the first word
                          .check(selectionMatches(0, 5));
}

/**
 * Checks that when doubleclicking a whole word is selected
 */
TEST_F(UITextField, DoubleClickWordSelection2) {
    By::type<ATextField>().perform(doubleClick({54_dp, 0_dp})) // click at the second word
                          .check(selectionMatches(6, 12));
}

/**
 * Checks cursor position when clicking between 'l' and 'o'.
 */
TEST_F(UITextField, CursorClickPos1) {
    By::type<ATextField>().perform(click({23_dp, 0_dp})) // hardcoded mouse position
            .check(selectionMatches(4));
}

TEST_F(UITextField, LeftRight) {
    By::type<ATextField>()
            .perform(click({0, 0}))
            .check(selectionMatches(0))
            .perform(keyDownAndUp(AInput::RIGHT))
            .check(selectionMatches(1))
            .perform(keyDownAndUp(AInput::LEFT))
            .check(selectionMatches(0))
            ;
}

TEST_F(UITextField, HomeEnd) {
    By::type<ATextField>()
            .perform(click({0, 0}))
            .check(selectionMatches(0))
            .perform(keyDownAndUp(AInput::END))
            .check(selectionMatches("hello world!"sv.length()))
            .perform(keyDownAndUp(AInput::HOME))
            .check(selectionMatches(0))
            ;
}

TEST_F(UITextField, CtrlA) {
    By::type<ATextField>()
            .perform(keyDownAndUp(AInput::LCONTROL + AInput::A))
            .perform(type("replace"))
            .check(text("replace"));
}

TEST_F(UITextField, CtrlLeftRight) {
    By::type<ATextField>()
            .perform(click({0, 0}))
            .check(selectionMatches(0))
            .perform(keyDownAndUp(AInput::LCONTROL + AInput::RIGHT))
            .check(selectionMatches("hello "sv.length()))
            .perform(keyDownAndUp(AInput::LCONTROL + AInput::LEFT))
            .check(selectionMatches(0))
            ;
}

TEST_F(UITextField, CtrlShiftLeftRight) {
    By::type<ATextField>()
            .perform(click({0, 0}))
            .check(selectionMatches(0))
            .perform(keyDownAndUp(AInput::LCONTROL + AInput::LSHIFT + AInput::RIGHT))
            .check(selectionMatches(0, "hello "sv.length()))
            .perform(keyDownAndUp(AInput::LCONTROL + AInput::LSHIFT + AInput::LEFT))
            .check(selectionMatches(0))
            ;
}

namespace {
enum class FieldHeight {
    /// Exactly AFontStyle::getTextBlockHeight().
    TIGHT,
    /// Much taller than the text.
    TALL,
};

/**
 * Shows a text field with "HHHH" and measures empty space above and below dark pixels (text or selection) in it.
 */
uitest_util::InkGaps measureTextField(float lineSpacing, VerticalAlign verticalAlign, FieldHeight height, bool selected) {
    constexpr int TALL_FIELD_HEIGHT = 300;
    auto field = _new<ATextField>() AUI_OVERRIDE_STYLE {
        FontSize { 80_pt },
        TextColor { 0x000000_rgb },
        BackgroundSolid { 0xffffff_rgb },
        Border { nullptr },
        LineHeight { lineSpacing },
        verticalAlign,
        Padding { 0 },
        Margin { 0 },
    };
    field->setText("HHHH");

    class TestWindow : public AWindow {
    public:
        explicit TestWindow(_<AView> v) {
            setContents(Vertical { std::move(v) } AUI_OVERRIDE_STYLE { BackgroundSolid { 0xffffff_rgb } });
            setSize({ 600, 800 });
        }
    };
    _new<TestWindow>(field)->show();
    uitest::frame();

    auto& fs = field->getFontStyle();
    field->setFixedSize({ 400, height == FieldHeight::TIGHT ? fs.getTextBlockHeight() : TALL_FIELD_HEIGHT });
    if (selected) {
        field->selectAll();
    }
    return uitest_util::measureInkGaps(field);
}
}   // namespace

/**
 * Checks that text field places the text in the same way as label does (see AFontStyle::getBaselineY).
 */
TEST_F(UITextField, VerticalAlignMiddleCentersText) {
    // tight field: text is centered thanks to the mirrored descender
    const auto reference = measureTextField(0.f, VerticalAlign::DEFAULT, FieldHeight::TIGHT, false);
    for (float lineSpacing : { 0.f, 0.5f, 1.f, 1.2f }) {
        const auto gaps = measureTextField(lineSpacing, VerticalAlign::MIDDLE, FieldHeight::TALL, false);
        EXPECT_NEAR(gaps.difference(), reference.difference(), 2)
            << "lineSpacing=" << lineSpacing << " top=" << gaps.top << " bottom=" << gaps.bottom;
    }
}

/**
 * Selection rectangle covers the same area as text does (capitals, descender and the descender mirrored above), thus
 * it must be centered in the field as well.
 */
TEST_F(UITextField, SelectionIsCenteredWithVerticalAlignMiddle) {
    for (float lineSpacing : { 0.f, 1.f }) {
        const auto gaps = measureTextField(lineSpacing, VerticalAlign::MIDDLE, FieldHeight::TALL, true);
        EXPECT_NEAR(gaps.difference(), 0, 2)
            << "lineSpacing=" << lineSpacing << " top=" << gaps.top << " bottom=" << gaps.bottom;
    }
}
