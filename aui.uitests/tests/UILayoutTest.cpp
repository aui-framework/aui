/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
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
#include "AUI/ASS/Property/Expanding.h"
#include "AUI/ASS/Property/FixedSize.h"
#include "AUI/ASS/Property/LayoutSpacing.h"
#include "AUI/Test/UI/By.h"
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
TEST_F(UILayoutTest, SmallCorner1) {
    class View: public ALabel {
    public:
        using ALabel::ALabel;
        void setGeometry(int x, int y, int width, int height) override {
            ALabel::setGeometry(x, y, width, height);
            auto box = By::name("Box").one();
            AUI_ASSERT((getCenterPointInWindow().x <= box->getPositionInWindow().x + box->getWidth()));
        }
    };

    auto cornerLabel = _new<View>("26") with_style {
            ATextAlign::RIGHT,
            FontSize{8_dp},
            LineHeight{9.68},
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

TEST_F(UILayoutTest, LayoutSpacing1) {
    inflate(Horizontal::Expanding {
        Button { "1" } with_style { Expanding{} },
    } with_style {
        LayoutSpacing{8_dp},
        FixedSize(200_dp, {})
    });
    auto b = By::type<AButtonEx>().one();
    
    // checks the buttons margins are perfectly equal
    auto parent = b->getParent();
    EXPECT_EQ((parent->getSize() - b->getSize()) / 2, b->getPosition());
}
TEST_F(UILayoutTest, LayoutSpacing2) {
    inflate(Horizontal::Expanding {
        Button { "1" } with_style { Expanding{} },
        Button { "2" } with_style { Expanding{} },
    } with_style {
        LayoutSpacing{8_dp},
        FixedSize(200_dp, {})
    });

    By::type<AButtonEx>() 
        .check(sameWidth(), "widths of the buttons are not equal")
        ;
}

TEST_F(UILayoutTest, LayoutSpacing3) {
    inflate(Horizontal::Expanding {
        Button { "1" } with_style { Expanding{} },
        Button { "2" } with_style { Expanding{} },
        Button { "3" } with_style { Expanding{} },
        Button { "4" } with_style { Expanding{} },
    } with_style {
        LayoutSpacing{8_dp},
        FixedSize(200_dp, {})
    });

    By::type<AButtonEx>() 
        .check(sameWidth(), "widths of the buttons are not equal")
        ;
}

namespace {
class LabelMock: public ALabel {
public:
    LabelMock(AString text): ALabel(std::move(text)) {
        ON_CALL(*this, getContentMinimumWidth).WillByDefault([this](const auto& a) {
            return ALabel::getContentMinimumWidth(a);
        });
    }
    MOCK_METHOD(int, getContentMinimumWidth, (ALayoutDirection layout), (override));
};
}

TEST_F(UILayoutTest, GetContentMinimumWidthPerformance1) {
    // checks how many times getContentMinimumWidth is called.
    // in this test, it should call be exactly once.

    testing::InSequence s;
    auto l = _new<LabelMock>("test");
    EXPECT_CALL(*l, getContentMinimumWidth(ALayoutDirection::HORIZONTAL)).Times(1);
    inflate(Centered { Horizontal {
        l,
    }});
    l->getWindow()->updateLayout();

    // extra layout update that should call LabelMock::getContentMinimumWidth one more time
    l->getWindow()->updateLayout();
}

TEST_F(UILayoutTest, GetContentMinimumWidthPerformance2) {
    // in contract to GetContentMinimumWidthPerformance1, here we additionaly change text of the label, effectively
    // forcing two layout updates.

    testing::InSequence s;
    auto l = _new<LabelMock>("test");
    EXPECT_CALL(*l, getContentMinimumWidth(ALayoutDirection::HORIZONTAL)).Times(2);
    inflate(Centered { Horizontal {
            l,
    }});
    l->getWindow()->updateLayout();
    l->setText("test2");
    l->getWindow()->updateLayout();

    // extra layout update that should call LabelMock::getContentMinimumWidth one more time
    l->getWindow()->updateLayout();
}
