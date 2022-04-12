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


class UIScrollTest: public testing::UITest {
public:
protected:

    class TestWindow: public AWindow {
    public:
        _<AScrollArea> mScrollArea;
        _<AViewContainer> mContainer;

        TestWindow() {
            setContents(Vertical {
                    mScrollArea = AScrollArea::Builder().withContents(
                            mContainer = Vertical {
                                    _new<ALabel>("Content"),
                                    _new<ALabel>("Content"),
                                    _new<ALabel>("Content"),
                                    _new<ALabel>("Content"),
                                    _new<ALabel>("Content"),
                                    _new<ALabel>("Content"),
                                    _new<ALabel>("Content"),
                                    _new<ALabel>("Content"),
                                    _new<ALabel>("Content"),
                                    _new<ALabel>("Content"),
                                    _new<ALabel>("Content"),
                                    _new<ALabel>("Content"),
                                    _new<ALabel>("Content"),
                                    _new<ALabel>("Content"),
                                    _new<ALabel>("Content"),
                                    _new<ALabel>("Content"),
                                    _new<ALabel>("Content"),
                                    _new<ALabel>("Content"),
                                    _new<ALabel>("Content"),
                                    _new<ALabel>("Content"),
                            } with_style { ass::BackgroundSolid{ 0xff0000_rgb, } } ).withExpanding().build()
            });

            pack();
        }
    };
    _<TestWindow> mTestWindow;


    void SetUp() override {
        UITest::SetUp();

        // prepare the window
        mTestWindow = _new<TestWindow>();
        mTestWindow->show();
    }

    void TearDown() override {
        UITest::TearDown();
    }
};



TEST_F(UIScrollTest, ContainedViewExpanded) {
    // scroll a little to see whether container view expanded properly
    mTestWindow->mScrollArea->scroll(0, 10);
    mTestWindow->updateLayout();
    By::type<AScrollArea>().perform(scroll({0, 10}));
    EXPECT_GE(mTestWindow->mContainer->getHeight(), mTestWindow->getHeight());
}
