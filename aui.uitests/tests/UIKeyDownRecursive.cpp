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