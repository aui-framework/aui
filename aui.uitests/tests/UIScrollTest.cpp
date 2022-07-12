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

#include <AUI/UITest.h>
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/AButton.h>
#include <AUI/View/ATextField.h>
#include <AUI/View/AScrollArea.h>
#include <gmock/gmock.h>

class MockedViewContainer: public AViewContainer {
public:
    MockedViewContainer() {
        ON_CALL(*this, setSize).WillByDefault([this](int width, int height) {
            AViewContainer::setSize(width, height);
        });

        setContents(
            Vertical {
                _new<ALabel>("Content") with_style { ass::MinSize{ {}, 300_dp } },
            }
        );
        *this << ".container";
        setCustomStyle({ ass::BackgroundSolid{ 0xff0000_rgb, } });

        EXPECT_CALL(*this, setSize(testing::_, testing::_)).Times(testing::AtLeast(1));
    }

    MOCK_METHOD(void, setSize, (int width, int height), (override));
};

class UIScrollTest: public testing::UITest {
public:
protected:

    class TestWindow: public AWindow {
    public:
        TestWindow(): AWindow("Test window", 200_dp, 100_dp) {
            setContents(Vertical {
                    AScrollArea::Builder().withContents(_new<MockedViewContainer>()).withExpanding().build()
            });
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
        // delete mocked objects
        mTestWindow->removeAllViews();
        AThread::processMessages();
        UITest::TearDown();
    }
};



TEST_F(UIScrollTest, ContainedViewExpanded) {
    // scroll a little to see whether container view expanded properly
    mTestWindow->updateLayout();
    By::type<AScrollArea>().perform(scroll({0, 500}));
    By::name(".container").check(uitest::impl::not$(bottomAboveBottomOf(By::type<AScrollArea>())));


}
