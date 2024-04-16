// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2024 Alex2772 and Contributors
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

        class TestWindow: public AWindow {
        private:
            _<ALabel> mHelloLabel;

        public:
            TestWindow() {
                setContents(Centered {
                        Vertical {
                                _new<AButton>("Say hello").connect(&AView::clicked, this, [&] {
                                    mHelloLabel->setVisibility(Visibility::VISIBLE);
                                    onButtonClicked();
                                }).connect(&AView::clickedRightOrLongPressed, this, [&] {
                                    onButtonClickedRightOrLongPressed();
                                }).connect(&AView::doubleClicked, this, [&] {
                                    onButtonDoubleClicked();
                                }) let { it->setDefault(); },
                                mHelloLabel = _new<ALabel>("Hello!") let { it->setVisibility(Visibility::INVISIBLE); }
                        }
                });

                pack();
            }

            MOCK_METHOD(void, onButtonClicked, (), ());
            MOCK_METHOD(void, onButtonDoubleClicked, (), ());
            MOCK_METHOD(void, onButtonClickedRightOrLongPressed, (), ());
        };

    void SetUp() override {
        UITest::SetUp();

        mTestWindow = _new<TestWindow>();
        mTestWindow->show();
    }

    void TearDown() override {
        mTestWindow = nullptr;
        UITest::TearDown();
    }

    _<TestWindow> mTestWindow;
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
    EXPECT_CALL(*mTestWindow, onButtonClicked).Times(1);
    // press the button
    By::text("Say hello").perform(click());

    // check label is appeared
    By::text("Hello!").check(visible(), "label is not appeared");
}

/**
 * Checks clickedRightOrLongPressed by right clicking.
 */
TEST_F(UIClick, RightClick) {
    // check clicked right or long pressed signal emitted
    EXPECT_CALL(*mTestWindow, onButtonClickedRightOrLongPressed).Times(1);

    // press the button
    By::text("Say hello").perform(clickRight());
}

/**
 * Checks clickedRightOrLongPressed by right clicking.
 */
TEST_F(UIClick, LongPress) {
    // check clicked right or long pressed signal is emitted
    EXPECT_CALL(*mTestWindow, onButtonClickedRightOrLongPressed).Times(1);

    // press the button
    By::text("Say hello").perform(gestureLongPress());
}

/**
 * Checks if clicks will be performed after onPointerPressed and onPointerReleased
 */
TEST_F(UIClick, Click) {
    // check clicked singal is emitted
    EXPECT_CALL(*mTestWindow, onButtonClicked).Times(1);

    mTestWindow->onPointerPressed({
        .position = By::text("Say hello").one()->getCenterPointInWindow(),
        .asButton = AInput::LBUTTON
    });

    mTestWindow->onPointerReleased({
        .position = By::text("Say hello").one()->getCenterPointInWindow(),
        .asButton = AInput::LBUTTON
    });
}

/**
 * Checks if double click signal will be emitted after two onPointerPressed and onPointerReleased
 */
TEST_F(UIClick, DoubleClickEmit) {
    testing::InSequence s;

    EXPECT_CALL(*mTestWindow, onButtonClicked).Times(2);
    EXPECT_CALL(*mTestWindow, onButtonDoubleClicked).Times(1);

    mTestWindow->onPointerPressed({
        .position = By::text("Say hello").one()->getCenterPointInWindow(),
        .asButton = AInput::LBUTTON
    });

    mTestWindow->onPointerReleased({
        .position = By::text("Say hello").one()->getCenterPointInWindow(),
        .asButton = AInput::LBUTTON
    });

    mTestWindow->onPointerPressed({
        .position = By::text("Say hello").one()->getCenterPointInWindow(),
        .asButton = AInput::LBUTTON
    });

    mTestWindow->onPointerReleased({
        .position = By::text("Say hello").one()->getCenterPointInWindow(),
        .asButton = AInput::LBUTTON
    });
}

/**
 * Checks if double click signal will not be emitted after two onPointerPressed and onPointerReleased with delay
 */
TEST_F(UIClick, DoubleClickNoEmit) {
    EXPECT_CALL(*mTestWindow, onButtonClicked).Times(2);
    EXPECT_CALL(*mTestWindow, onButtonDoubleClicked).Times(0);

    mTestWindow->onPointerPressed({
        .position = By::text("Say hello").one()->getCenterPointInWindow(),
        .asButton = AInput::LBUTTON
    });

    mTestWindow->onPointerReleased({
        .position = By::text("Say hello").one()->getCenterPointInWindow(),
        .asButton = AInput::LBUTTON
    });

    AThread::sleep(ABaseWindow::DOUBLECLICK_MAX_DURATION * 2);

    mTestWindow->onPointerPressed({
        .position = By::text("Say hello").one()->getCenterPointInWindow(),
        .asButton = AInput::LBUTTON
    });

    mTestWindow->onPointerReleased({
        .position = By::text("Say hello").one()->getCenterPointInWindow(),
        .asButton = AInput::LBUTTON
    });
}
