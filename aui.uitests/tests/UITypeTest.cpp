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
#include <AUI/View/ATextField.h>

/**
 * There's a text field and a button:
 * _____________________-[]X_
 * |                        |
 * |  [       ][Say hello]  |
 * |                        |
 * |________________________|
 *
 *
 * user types his name. When it's button is pressed, a message appears:
 * _____________________-[]X_
 * |                        |
 * |  [Steve  ][Say hello]  |
 * |      Hello, Steve!     |
 * |________________________|
 *
 * when text field is empty, hello message should be hidden.
 */

class UIType: public testing::UITest {
public:
protected:
    void SetUp() override {
        UITest::SetUp();

        class TestWindow: public AWindow {
        private:
            _<ATextField> mTextField;
            _<ALabel> mHelloLabel;

        public:
            TestWindow() {
                setContents(Centered {
                        Vertical {
                                Horizontal {
                                        mTextField = _new<ATextField>() << "#username",
                                        _new<AButton>("Say hello").connect(&AView::clicked, this, [&] {
                                            if (mTextField->text().empty()) {
                                                mHelloLabel->setVisibility(Visibility::INVISIBLE);
                                                return;
                                            }
                                            mHelloLabel->setText("Hello, {}!"_format(mTextField->text()));
                                            mHelloLabel->setVisibility(Visibility::VISIBLE);
                                        }) let { it->setDefault(); },
                                },
                                mHelloLabel = _new<ALabel>() let {
                                    it->setVisibility(Visibility::INVISIBLE);
                                    it << "#hello";
                                }
                        }
                });

                pack();
            }
        };

        // prepare the window
        _new<TestWindow>()->show();
    }

    void TearDown() override {
        UITest::TearDown();
    }
};



/**
 * Checks that the message is not appeared yet.
 */
TEST_F(UIType, HelloIsNotAppeared) {

    // check label is not visible
    By::type<ALabel>().check(notVisible(), "label is visible");
}

/**
 * Checks that the message does not appear when button is clicked.
 */
TEST_F(UIType, HelloNotAppearsAfterClick) {
    // press the button
    By::text("Say hello").perform(click());

    // check label is NOT appeared
    By::name("#hello").check(notVisible(), "label is appeared");
}

/**
 * Checks that the message does not appear when button is clicked.
 *
 * 1. types "Steve" to the text field
 * 2. clicks the button
 */
TEST_F(UIType, HelloAppearsAfterClick) {
    // type "Steve" to the text field
    By::name("#username").perform(type("Steve"));

    // press the button
    By::text("Say hello").perform(click());

    // check label is appeared and contains text "Hello, Steve!"
    By::name("#hello").check(visible(), "label is not appeared")
                      .check(text("Hello, Steve!"), "invalid text");
}

/**
 * Checks that the message disappears.
 *
 * 1. types "Steve" to the text field
 * 2. clicks the button
 * 3. clears the text field
 * 4. clicks the button again
 */
TEST_F(UIType, HelloDisappearsAfterClick) {
    // type "Steve" to the text field
    By::name("#username").perform(type("Steve"));

    // press the button
    By::text("Say hello").perform(click());

    // check label is appeared and contains text "Hello, Steve!"
    By::name("#hello").check(visible(), "label is not appeared")
                            .check(text("Hello, Steve!"), "invalid text");
}