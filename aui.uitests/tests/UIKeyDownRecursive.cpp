/*
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

#include <gmock/gmock.h>
#include <AUI/UITest.h>
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/AButton.h>
#include <AUI/View/ATextField.h>


namespace {
    class InnerDialogBox : public AViewContainer {
    public:
        InnerDialogBox(const _<AViewContainer>& contents) {
            setContents(contents);
        }

        MOCK_METHOD(void, onKeyDown, (AInput::Key key), (override));
    };

    class TestWindow : public AWindow {
    public:
        _<ATextField> mTextField;
        _<InnerDialogBox> mInnerDialogBox;

        TestWindow() {
            setContents(Centered{
                    mInnerDialogBox = _new<InnerDialogBox>(Vertical{
                            Horizontal{
                                    mTextField = _new<ATextField>() << "#username",
                                    _new<AButton>("Say hello"),
                            },
                    })
            });

            pack();
        }
    };
}

class UIKeyDownRecursive: public testing::UITest {
public:
protected:
    void SetUp() override {
        UITest::SetUp();

        // prepare the window
        (mTestWindow = _new<TestWindow>())->show();

    }

    void TearDown() override {
        // delete mocked objects
        mTestWindow->close();
        mTestWindow->removeAllViews();
        mTestWindow = nullptr;
        AThread::processMessages();
        UITest::TearDown();
    }

    _<TestWindow> mTestWindow;

};



/**
 * Checks that when the text field focused and enter pressed, the inner dialog box handles that message.
 */
TEST_F(UIKeyDownRecursive, TextField) {
    EXPECT_CALL(*mTestWindow->mInnerDialogBox, onKeyDown(AInput::RETURN)).Times(1);

    By::type<ATextField>().perform(keyDownAndUp(AInput::RETURN));
}