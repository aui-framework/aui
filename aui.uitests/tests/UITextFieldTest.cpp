// AUI Framework - Declarative UI toolkit for modern C++17
// Copyright (C) 2020-2023 Alex2772
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

#include <AUI/UITest.h>
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/ATextField.h>

using namespace ass;

std::ostream& operator<<(std::ostream& o, const ACursorSelectable::Selection& e) noexcept{
    o << "Selection";
    if (e.end == -1) {
        o << "{" << e.begin << "}";
    } else {
        o << "(" << e.begin << ";" << e.end << "]";
    }
    return o;
}


class UITextField: public testing::UITest {
public:
protected:
    void SetUp() override {
        UITest::SetUp();

        class TestWindow: public AWindow {
        public:
            TestWindow() {
                setContents(Centered {
                        _new<ATextField>() with_style { FixedSize { 300_dp, {} } } let {
                            it->setText("hello world!");
                        }
                });
                pack();
            }
        };

        _new<TestWindow>()->show();
    }

    void TearDown() override {
        UITest::TearDown();
    }
};

struct SelectionMatchesAssert {
    ACursorSelectable::Selection selection;

    SelectionMatchesAssert(unsigned begin, unsigned end) : selection{begin, end} {}
    SelectionMatchesAssert(unsigned s) : selection{s, static_cast<unsigned>(-1)} {}

    bool operator()(const _<AView>& view) const {
        if (auto t = _cast<ACursorSelectable>(view)) {
            EXPECT_EQ(t->selection(), selection);
        }
        return true;
    }
};

using selectionMatches = SelectionMatchesAssert;

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
/**
 * Checks cursor position when clicking between 'o' and 'r'.
 */
TEST_F(UITextField, CursorClickPos2) {
    By::type<ATextField>().perform(click({51_dp, 0_dp})) // hardcoded mouse position
            .check(selectionMatches(8));
}
