/**
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
 */

#include <AUI/UITest.h>
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/AButton.h>
#include <AUI/View/ATextField.h>
#include <AUI/View/AScrollArea.h>


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
                setContents(Vertical {
                    AScrollArea::Builder().withContents(
                            mConsoleDisplayPort = Vertical {
                                    _new<ALabel>("Hacker's MC Launcher")
                            } << ".console_wrap").withExpanding().build()
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
    By::text("Hello!").check(notVisible(), "label is visible");
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