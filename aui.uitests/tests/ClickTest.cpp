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

using namespace boost::unit_test;

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
BOOST_AUTO_TEST_SUITE(Click)

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



/**
 * Checks that the message is not appeared yet.
 */
UI_TEST_CASE(HelloIsNotAppeared) {
    // prepare the window
    _new<TestWindow>()->show();

    // check label is not visible
    By::text("Hello!").require(notVisible(), "label is visible");
}

/**
 * Checks that the message appears when button is clicked.
 */
UI_TEST_CASE(HelloAppearsAfterClick) {
    // prepare the window
    _new<TestWindow>()->show();


    // press the button
    By::text("Say hello").perform(click());

    // check label is appeared
    By::text("Hello!").require(visible(), "label is not appeared");
}
BOOST_AUTO_TEST_SUITE_END()