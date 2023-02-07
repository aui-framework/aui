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

#include <AUI/UITest.h>
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/AButton.h>
#include <AUI/View/ATextField.h>
#include <AUI/View/AScrollArea.h>
#include <gmock/gmock.h>

class MockedViewContainer: public AViewContainer {
public:
    MockedViewContainer() {
        ON_CALL(*this, setSize).WillByDefault([this](glm::ivec2 size) {
            AViewContainer::setSize(size);
        });

        setContents(
            Vertical {
                _new<ALabel>("Content") with_style { ass::MinSize{ {}, 300_dp } },
            }
        );
        *this << ".container";
        setCustomStyle({ ass::BackgroundSolid{ 0xff0000_rgb, } });

        EXPECT_CALL(*this, setSize(testing::_)).Times(testing::AtLeast(1));
    }

    MOCK_METHOD(void, setSize, (glm::ivec2), (override));
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
    By::name(".container").check(bottomAboveBottomOf(By::type<AScrollArea>()));


}
