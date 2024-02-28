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

#include <AUI/UITest.h>
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/AButton.h>
#include <AUI/View/ATextField.h>

using namespace ass;

/**
 * This suite tests aui::uitests' ability to determine what's going on on the screen.
 */
class UIScreenAnalyzer: public testing::UITest {
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
                setContents(Centered {
                        Vertical {
                                Horizontal {
                                        mTextField = _new<ATextField>() << "#username",
                                        _new<AButton>("Say hello").connect(&AView::clicked, this, [&] {
                                            if (mTextField->text().empty()) {
                                                mHelloLabel->setVisibility(Visibility::INVISIBLE);
                                                return;
                                            }
                                            mHelloLabel->setText("Hello, {}!"_format(mTextField->text()));
                                            mHelloLabel->setVisibility(Visibility::VISIBLE);
                                        }) with_style { BackgroundSolid { 0xff0000_rgb } } let { it->setDefault(); },
                                },
                                mHelloLabel = _new<ALabel>() let {
                                    it->setVisibility(Visibility::INVISIBLE);
                                    it << "#hello";
                                },
                                _new<AButton>("A basic button")
                        }
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
 * Checks averageColor.
 */
TEST_F(UIScreenAnalyzer, AverageColor) {
    // check the button is red
    By::text("Say hello").check(averageColor(0xff0000_rgb), "the button is not red");
    By::text("A basic button").check(uitest::impl::not$(averageColor(0xff0000_rgb)), "the button is red");
}
