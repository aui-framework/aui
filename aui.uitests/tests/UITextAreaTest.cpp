/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <AUI/UITest.h>
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/ATextArea.h>
#include "AUI/View/ATextArea.h"

using namespace ass;
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

class UITextArea: public testing::UITest {
public:
    ~UITextArea() override = default;

protected:
    void SetUp() override {
        UITest::SetUp();


        auto window = _new<AWindow>();
        window->setContents(Centered {
            mTextArea
        });
        window->pack();
        window->show();
        uitest::frame();
    }

    _<ATextArea> mTextArea = _new<ATextArea>() with_style { FixedSize { 300_dp, {} } } let {
        it->setText("hello world!");
    };

    void TearDown() override {
        UITest::TearDown();
    }


    auto& entries() {
        return mTextArea->mEngine.entries();
    }
};

/**
 * Checks that then clicking at the left border of the text field, cursor jumps to the first symbol.
 */
TEST_F(UITextArea, CursorAppearsAtTheStart) {
    By::type<ATextArea>().perform(click({0_dp, 0_dp})) // click at the left border
                          .check(selectionMatches(0));
}

/**
 * Checks that then clicking at the right border of the text field, cursor jumps to the last symbol.
 */
TEST_F(UITextArea, CursorAppearsAtTheEnd) {
    By::type<ATextArea>().perform(click({299_dp, 0_dp})) // click at the right border
                          .check(selectionMatches(12));
}

/**
 * Checks that when doubleclicking a whole word is selected
 */
TEST_F(UITextArea, DoubleClickWordSelection1) {
    By::type<ATextArea>().perform(doubleClick({20_dp, 0_dp})) // click at the first word
                          .check(selectionMatches(0, 5));
}

/**
 * Checks that when doubleclicking a whole word is selected
 */
TEST_F(UITextArea, DoubleClickWordSelection2) {
    By::type<ATextArea>().perform(doubleClick({54_dp, 0_dp})) // click at the second word
                          .check(selectionMatches(6, 12));
}

/**
 * Checks cursor position when clicking between 'l' and 'o'.
 */
TEST_F(UITextArea, CursorClickPos1) {
    By::type<ATextArea>().perform(click({23_dp, 0_dp})) // hardcoded mouse position
            .check(selectionMatches(4));
}

/**
 * Checks cursor position when clicking between 'o' and 'r'.
 */
TEST_F(UITextArea, CursorClickPos2) {
    By::type<ATextArea>().perform(click({51_dp, 0_dp})) // hardcoded mouse position
            .check(selectionMatches(8));
}

TEST_F(UITextArea, LeftRight) {
    By::type<ATextArea>()
            .perform(click({0, 0}))
            .check(selectionMatches(0))
            .perform(keyDownAndUp(AInput::RIGHT))
            .check(selectionMatches(1))
            .perform(keyDownAndUp(AInput::LEFT))
            .check(selectionMatches(0))
            ;
}

TEST_F(UITextArea, HomeEnd) {
    By::type<ATextArea>()
            .perform(click({0, 0}))
            .check(selectionMatches(0))
            .perform(keyDownAndUp(AInput::END))
            .check(selectionMatches("hello world!"sv.length()))
            .perform(keyDownAndUp(AInput::HOME))
            .check(selectionMatches(0))
            ;
}

TEST_F(UITextArea, CtrlA) {
    By::type<ATextArea>()
            .perform(keyDownAndUp(AInput::LCONTROL + AInput::A))
            .perform(type("replace"))
            .check(text("replace"));
}

TEST_F(UITextArea, CtrlLeftRight) {
    By::type<ATextArea>()
            .perform(click({0, 0}))
            .check(selectionMatches(0))
            .perform(keyDownAndUp(AInput::LCONTROL + AInput::RIGHT))
            .check(selectionMatches("hello "sv.length()))
            .perform(keyDownAndUp(AInput::LCONTROL + AInput::LEFT))
            .check(selectionMatches(0))
            ;
}

TEST_F(UITextArea, CtrlShiftLeftRight) {
    By::type<ATextArea>()
            .perform(click({0, 0}))
            .check(selectionMatches(0))
            .perform(keyDownAndUp(AInput::LCONTROL + AInput::LSHIFT + AInput::RIGHT))
            .check(selectionMatches(0, "hello "sv.length()))
            .perform(keyDownAndUp(AInput::LCONTROL + AInput::LSHIFT + AInput::LEFT))
            .check(selectionMatches(0))
            ;
}

TEST_F(UITextArea, Type1) {
    mTextArea->clear();
    EXPECT_EQ(mTextArea->text(), "");
    By::type<ATextArea>()
        .perform(type("aui"))
        .check(selectionMatches(3))
        ;
    EXPECT_EQ(mTextArea->text(), "aui");
    EXPECT_EQ(entries().size(), 1);
}

TEST_F(UITextArea, Type2a) {
    mTextArea->clear();
    EXPECT_EQ(mTextArea->text(), "");
    By::type<ATextArea>()
            .perform(type("aui "))
            .check(selectionMatches(4))
            ;
    EXPECT_EQ(mTextArea->text(), "aui ");
    EXPECT_EQ(entries().size(), 2);
}

TEST_F(UITextArea, Type2b) {
    mTextArea->clear();
    EXPECT_EQ(mTextArea->text(), "");
    By::type<ATextArea>()
            .perform(type(" aui"))
            .check(selectionMatches(4))
            ;
    EXPECT_EQ(mTextArea->text(), " aui");
    EXPECT_EQ(entries().size(), 2);
}

TEST_F(UITextArea, Type2c) {
    mTextArea->clear();
    EXPECT_EQ(mTextArea->text(), "");
    By::type<ATextArea>().perform(type("aui"));
    EXPECT_EQ(mTextArea->text(), "aui");
    mTextArea->setSelection(0);
    By::type<ATextArea>().perform(type(" "));
    EXPECT_EQ(mTextArea->text(), " aui");
    EXPECT_EQ(entries().size(), 2);
}

TEST_F(UITextArea, Type2d) {
    mTextArea->clear();
    EXPECT_EQ(mTextArea->text(), "");
    By::type<ATextArea>().perform(type("aui"));
    EXPECT_EQ(mTextArea->text(), "aui");
    mTextArea->setSelection(1);
    By::type<ATextArea>().perform(type(" "));
    EXPECT_EQ(mTextArea->text(), "a ui");
    EXPECT_EQ(entries().size(), 3);
}

TEST_F(UITextArea, Type3) {
    mTextArea->clear();
    EXPECT_EQ(mTextArea->text(), "");
    By::type<ATextArea>()
            .perform(type("aui framework"))
            .check(selectionMatches(13))
            ;
    EXPECT_EQ(mTextArea->text(), "aui framework");
    EXPECT_EQ(entries().size(), 3);
}

TEST_F(UITextArea, Type4) {
    mTextArea->clear();
    EXPECT_EQ(mTextArea->text(), "");
    By::type<ATextArea>()
            .perform(type("aui  framework"))
            .check(selectionMatches(14))
            ;
    EXPECT_EQ(mTextArea->text(), "aui  framework");
    EXPECT_EQ(entries().size(), 3);
}

TEST_F(UITextArea, Type5) {
    mTextArea->clear();
    EXPECT_EQ(mTextArea->text(), "");
    By::type<ATextArea>().perform(type("   "));
    EXPECT_EQ(mTextArea->text(), "   ");
    EXPECT_EQ(entries().size(), 1);
    mTextArea->setSelection(1);
    By::type<ATextArea>().perform(type("aui"));
    EXPECT_EQ(mTextArea->text(), " aui  ");
    EXPECT_EQ(entries().size(), 3);
}
