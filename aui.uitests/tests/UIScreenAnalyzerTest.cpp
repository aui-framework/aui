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
                                            if (mTextField->text()->empty()) {
                                                mHelloLabel->setVisibility(Visibility::INVISIBLE);
                                                return;
                                            }
                                            mHelloLabel->text() = "Hello, {}!"_format(*mTextField->text());
                                            mHelloLabel->setVisibility(Visibility::VISIBLE);
                                        }) AUI_WITH_STYLE { BackgroundSolid { 0xff0000_rgb } } AUI_LET { it->setDefault(); },
                                },
                                mHelloLabel = _new<ALabel>() AUI_LET {
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
