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
#include "AUI/ASS/Property/Expanding.h"
#include "AUI/ASS/Property/FixedSize.h"
#include "AUI/ASS/Property/LayoutSpacing.h"
#include "AUI/Test/UI/By.h"
#include "AUI/Util/ALayoutInflater.h"
#include "AUI/View/AGroupBox.h"

using namespace declarative;

namespace {

class UILayoutTest : public testing::UITest {
public:

protected:
    void SetUp() override { UITest::SetUp(); }

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
}   // namespace

// Checks for bug where cornerLabel goes outside of box.
TEST_F(UILayoutTest, SmallCorner1) {
    class View : public ALabel {
    public:
        using ALabel::ALabel;
        void setGeometry(int x, int y, int width, int height) override {
            ALabel::setGeometry(x, y, width, height);
            auto box = By::name("Box").one();
            AUI_ASSERT((getCenterPointInWindow().x <= box->getPositionInWindow().x + box->getWidth()));
        }
    };

    auto cornerLabel = _new<View>("26") AUI_OVERRIDE_STYLE {
        ATextAlign::RIGHT,
        FontSize { 8_dp },
        LineHeight { 9.68 },
        MinSize(12_dp, 8_dp),
        Padding(0),
        Margin(0),
        BackgroundSolid(0xff00ff_rgb),
    };
    _<AView> box =
        Horizontal {
            SpacerExpanding() AUI_OVERRIDE_STYLE { MinSize(0) },
            Vertical {
              SpacerExpanding() AUI_OVERRIDE_STYLE { MinSize(0) },
              cornerLabel,
            } AUI_OVERRIDE_STYLE { MinSize(0) }
        }
        << "Box" AUI_OVERRIDE_STYLE {
               FixedSize(22_dp),
               BackgroundSolid(0xffffff_rgb),
           };

    inflate(Centered::Expanding { box });

    (By::value(cornerLabel) | By::value(box))
        .check(areRightAligned(), "box and label are not right aligned")
        .check(areBottomAligned(), "box and label are not bottom aligned");
}

TEST_F(UILayoutTest, LayoutSpacing1) {
    inflate(Horizontal::Expanding {
      Button { Label { "1" } } AUI_OVERRIDE_STYLE { Expanding {} },
    } AUI_OVERRIDE_STYLE { LayoutSpacing { 8_dp }, FixedSize(200_dp, {}) });
    auto b = By::type<AButton>().one();

    // checks the buttons margins are perfectly equal
    auto parent = b->getParent();
    EXPECT_EQ((parent->getSize() - b->getSize()) / 2, b->getPosition());
}
TEST_F(UILayoutTest, LayoutSpacing2) {
    inflate(Horizontal::Expanding {
      Button { Label { "1" } } AUI_OVERRIDE_STYLE { Expanding {} },
      Button { Label { "2" } } AUI_OVERRIDE_STYLE { Expanding {} },
    } AUI_OVERRIDE_STYLE { LayoutSpacing { 8_dp }, FixedSize(200_dp, {}) });

    By::type<AButton>().check(sameWidth(), "widths of the buttons are not equal");
}

TEST_F(UILayoutTest, LayoutSpacing3) {
    inflate(Horizontal::Expanding {
      Button { Label { "1" } } AUI_OVERRIDE_STYLE { Expanding {} },
      Button { Label { "2" } } AUI_OVERRIDE_STYLE { Expanding {} },
      Button { Label { "3" } } AUI_OVERRIDE_STYLE { Expanding {} },
      Button { Label { "4" } } AUI_OVERRIDE_STYLE { Expanding {} },
    } AUI_OVERRIDE_STYLE { LayoutSpacing { 8_dp }, FixedSize(200_dp, {}) });

    By::type<AButton>().check(sameWidth(), "widths of the buttons are not equal");
}

TEST_F(UILayoutTest, LayoutSpacing4) {
    inflate(Vertical::Expanding {
        Button { Label { "1" } } AUI_OVERRIDE_STYLE {},
        Button { Label { "2" } } AUI_OVERRIDE_STYLE { Expanding {} },
    } AUI_OVERRIDE_STYLE { LayoutSpacing { -8_dp }, FixedSize(200_dp, {}) });

    uitest::frame();
    auto i = By::type<AButton>().toVector();
    EXPECT_GT(i[0]->getPosition().y + i[0]->getSize().y, i[1]->getPosition().y);
    EXPECT_LT(mWindow->getContentMinimumHeight(), i[0]->getMinimumHeight() + i[1]->getMinimumHeight());
}

TEST_F(UILayoutTest, LayoutSpacing5) {
    auto groupBox =
        GroupBox {
            Label { "Test" },
            Vertical {
                Label { "Test" },
            }
        } AUI_OVERRIDE_STYLE { Expanding {} };
    inflate(Centered::Expanding {
            groupBox
    } AUI_OVERRIDE_STYLE { FixedSize(200_dp, {}) });

    uitest::frame();
    auto title = groupBox->getViews()[0];
    auto body = groupBox->getViews()[1];
    EXPECT_GT(title->position()->y + title->size()->y, body->position()->y);
}

TEST_F(UILayoutTest, ExpandingWithMinSize1) {
    inflate(Horizontal::Expanding {
      _new<AView>() << ".expanding_minsize" AUI_OVERRIDE_STYLE { Expanding {}, MinSize { 200_dp, {} } },
      _new<AView>() << ".expanding" AUI_OVERRIDE_STYLE { Expanding {} },
    } AUI_OVERRIDE_STYLE { FixedSize(300_dp, {}) });

    By::name(".expanding_minsize").check(width(200_dp), "width of .expanding_minsize is invalid");
    By::name(".expanding").check(width(100_dp), "width of .expanding is invalid");
}

TEST_F(UILayoutTest, ExpandingWithMinSize2) {
    inflate(Horizontal::Expanding {
      _new<AView>() << ".expanding" AUI_OVERRIDE_STYLE { Expanding {} },
      _new<AView>() << ".expanding_minsize" AUI_OVERRIDE_STYLE { Expanding {}, MinSize { 200_dp, {} } },
    } AUI_OVERRIDE_STYLE { FixedSize(300_dp, {}) });

    By::name(".expanding_minsize").check(width(200_dp), "width of .expanding_minsize is invalid");
    By::name(".expanding").check(width(100_dp), "width of .expanding is invalid");
}

TEST_F(UILayoutTest, ExpandingWithMaxSize1) {
    inflate(Horizontal::Expanding {
        _new<AView>() << ".expanding_maxsize" AUI_OVERRIDE_STYLE { Expanding {}, MaxSize { 100_dp, {} } },
        _new<AView>() << ".expanding" AUI_OVERRIDE_STYLE { Expanding {} },
    } AUI_OVERRIDE_STYLE { FixedSize(300_dp, {}) });

    By::name(".expanding_maxsize").check(width(100_dp), "width of .expanding_maxsize is invalid");
    By::name(".expanding").check(width(200_dp), "width of .expanding is invalid");
}

TEST_F(UILayoutTest, ExpandingWithMaxSize2) {
    inflate(Horizontal::Expanding {
        _new<AView>() << ".expanding" AUI_OVERRIDE_STYLE { Expanding {} },
        _new<AView>() << ".expanding_maxsize" AUI_OVERRIDE_STYLE { Expanding {}, MaxSize { 100_dp, {} } },
    } AUI_OVERRIDE_STYLE { FixedSize(300_dp, {}) });

    By::name(".expanding_maxsize").check(width(100_dp), "width of .expanding_maxsize is invalid");
    By::name(".expanding").check(width(200_dp), "width of .expanding is invalid");
}

namespace {
class LabelMock : public ALabel {
public:
    LabelMock(AString text) : ALabel(std::move(text)) {
        ON_CALL(*this, getContentMinimumWidth).WillByDefault([this]() { return ALabel::getContentMinimumWidth(); });
    }
    MOCK_METHOD(int, getContentMinimumWidth, (), (override));
};
}   // namespace

TEST_F(UILayoutTest, GetContentMinimumWidthPerformance1) {
    // checks how many times getContentMinimumWidth is called.
    // in this test, it should call be exactly once.

    testing::InSequence s;
    auto l = _new<LabelMock>("test");
    EXPECT_CALL(*l, getContentMinimumWidth()).Times(1);
    inflate(Centered { Horizontal {
      l,
    } });
    l->getWindow()->applyGeometryToChildrenIfNecessary();

    // extra layout update that should not call LabelMock::getContentMinimumWidth one more time
    AUI_REPEAT(10) { l->getWindow()->applyGeometryToChildrenIfNecessary(); }
}

TEST_F(UILayoutTest, GetContentMinimumWidthPerformance2) {
    // in contract to GetContentMinimumWidthPerformance1, here we additionally change text of the label, effectively
    // forcing two layout updates.

    testing::InSequence s;
    auto l1 = _new<LabelMock>("test");
    auto l2 = _new<ALabel>("test");
    EXPECT_CALL(*l1, getContentMinimumWidth()).Times(2);
    inflate(Centered { Horizontal {
      l1,
      l2,
    } });
    l1->getWindow()->applyGeometryToChildrenIfNecessary();
    auto prevPosX = l2->getPositionInWindow().x;
    l1->text() = "test2";
    l1->getWindow()->applyGeometryToChildrenIfNecessary();

    EXPECT_GE(l2->getPositionInWindow().x, prevPosX);   // l2 is expected to shift to right.

    // extra layout update that should call LabelMock::getContentMinimumWidth one more time
    l1->getWindow()->applyGeometryToChildrenIfNecessary();
}
