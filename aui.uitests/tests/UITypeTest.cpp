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
                                            if (mTextField->getText().empty()) {
                                                mHelloLabel->setVisibility(Visibility::INVISIBLE);
                                                return;
                                            }
                                            mHelloLabel->text() = "Hello, {}!"_format(mTextField->getText());
                                            mHelloLabel->setVisibility(Visibility::VISIBLE);
                                        }) AUI_LET { it->setDefault(); },
                                },
                                mHelloLabel = _new<ALabel>() AUI_LET {
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