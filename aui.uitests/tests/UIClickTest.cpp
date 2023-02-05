// AUI Framework - Declarative UI toolkit for modern C++20
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
#include <AUI/View/AButton.h>

/**
 * This test suite checks a simple program.
 * There's a button "Say hello":
 * ___________-[]X_
 * |              |
 * |  [Say hello] |
 * |              |
 * |______________|
 *
 *
 * when it's pressed, a message appears:
 * ___________-[]X_
 * |              |
 * |  [Say hello] |
 * |    Hello!    |
 * |______________|
 *
 * that's it.
 */

class UIClick: public testing::UITest {
public:
protected:
    void SetUp() override {
        UITest::SetUp();

        class TestWindow: public AWindow {
        private:
            _<ALabel> mHelloLabel;

        public:
            TestWindow() {
                setContents(Centered {
                        Vertical {
                                _new<AButton>("Say hello").connect(&AView::clicked, this, [&] {
                                    mHelloLabel->setVisibility(Visibility::VISIBLE);
                                }) let { it->setDefault(); },
                                mHelloLabel = _new<ALabel>("Hello!") let { it->setVisibility(Visibility::INVISIBLE); }
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



/**
 * Checks that the message is not appeared yet.
 */
TEST_F(UIClick, HelloIsNotAppeared) {
    // check label is not visible
    By::text("Hello!").check(notVisible(), "label is visible");
}

/**
 * Checks that the message appears when button is clicked.
 */
TEST_F(UIClick, HelloAppearsAfterClick) {
    // press the button
    By::text("Say hello").perform(click());

    // check label is appeared
    By::text("Hello!").check(visible(), "label is not appeared");
}
