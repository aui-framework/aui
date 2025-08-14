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

#include <gmock/gmock.h>
#include <AUI/UITest.h>
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/AButton.h>
#include <AUI/View/ATextField.h>


using namespace declarative;

namespace {
    class InnerDialogBox : public AViewContainerBase {
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