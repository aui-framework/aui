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
#include <ExampleWindow.h>
#include <AUI/View/ATextField.h>

/**
 * <p>
 *   A bug appeared in this example application related to AUI layout managers. This suite checks the following issues:
 * </p>
 * <ul>
 *   <li>height of the latest element sometimes (not always!) seems to be incorrect</li>
 *   <li>latest elements sometimes (again, not always!) seems to be not fit to the parent's border</li>
 * </ul>
 *
 * <p>Details: <a href="https://github.com/aui-framework/aui/issues/18">https://github.com/aui-framework/aui/issues/18</a></p>
 * <p>Photo:</p>
 * <img src="https://user-images.githubusercontent.com/19491414/144756936-173e5d65-9433-4c00-92d6-142ca217164d.png" />
 *
 */


class UILayoutManager: public testing::UITest {
public:
protected:
    void SetUp() override {
        UITest::SetUp();
        _new<ExampleWindow>()->show();
    }

    void TearDown() override {
        UITest::TearDown();
    }
};


/**
 * Checks height of all buttons - they should be the same (see "Show all views..." button above).
 */
TEST_F(UILayoutManager, Height) {
    // check height
    (By::text("Common button") | By::text("Show all views...")).check(sameHeight(), "height mismatch");
}


/**
 * Checks alignment (see "Alex2772, 2021, alex2772.ru" - it is not perfectly aligned)
 */
TEST_F(UILayoutManager, LastElementAlignment) {
    // copyright width can be also not minimal
    By::name("#copyright").check(widthIsMinimal(), "copyright width should be minimal");
}

/**
 * Checks alignment (looks like buttons and list views are not perfectly aligned)
 */
TEST_F(UILayoutManager, ButtonsAlignment) {
    // buttons column should be perfectly aligned
    By::name("Common button")
        .parent()
        .allChildren()
        .check(leftRightAligned(), "elements should be perfectly aligned");
}
