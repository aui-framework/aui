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
#include "AUI/Util/ALayoutInflater.h"

using namespace declarative;

namespace {

    class UILayoutTest : public testing::UITest {
    public:
    protected:
        void SetUp() override {
            UITest::SetUp();
        }

        void inflate(_<AView> view) {
            mWindow = _new<AWindow>();
            using namespace declarative;
            ALayoutInflater::inflate(mWindow, std::move(view));
            mWindow->show();
        }

        void TearDown() override {
            mWindow = nullptr;
            mView = nullptr;
            UITest::TearDown();
        }

        _<AWindow> mWindow;
        _<AView> mView;
    };
}


// Checks for bug where cornerLabel goes outside of box.
TEST_F(UILayoutTest, SmallCorner) {
    class View: public ALabel {
    public:
        using ALabel::ALabel;
        void setGeometry(int x, int y, int width, int height) override {
            ALabel::setGeometry(x, y, width, height);
            auto box = By::name("Box").one();
            assert((getCenterPointInWindow().x <= box->getPositionInWindow().x + box->getWidth()));
        }
    };

    auto cornerLabel = _new<View>("26") with_style {
            TextAlign::RIGHT,
            FontSize(8_dp),
            LineHeight(9.68),
            MinSize(12_dp,8_dp),
            Padding(0),
            Margin(0),
            BackgroundSolid(0xff00ff_rgb),
    };
    _<AView> box = Horizontal {
        SpacerExpanding () with_style { MinSize(0) },
        Vertical {
            SpacerExpanding() with_style { MinSize(0) },
            cornerLabel,
        } with_style { MinSize(0) }
    } with_style {
        FixedSize(22_dp),
        BackgroundSolid(0xffffff_rgb),
    } << "Box";

    inflate(Centered::Expanding {
        box
    });

    (By::value(cornerLabel) | By::value(box))
        .check(areRightAligned(), "box and label are not right aligned")
        .check(areBottomAligned(), "box and label are not bottom aligned")
        ;
}
