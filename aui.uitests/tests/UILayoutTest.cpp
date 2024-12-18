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

    auto cornerLabel = _new<View>("26") with_style {
        ATextAlign::RIGHT, FontSize { 8_dp }, LineHeight { 9.68 },           MinSize(12_dp, 8_dp),
        Padding(0),        Margin(0),         BackgroundSolid(0xff00ff_rgb),
    };
    _<AView> box = Horizontal {
        SpacerExpanding() with_style { MinSize(0) }, Vertical {
            SpacerExpanding() with_style { MinSize(0) },
            cornerLabel,
        } with_style {
            MinSize(0)
        }
    }
    with_style {
        FixedSize(22_dp),
        BackgroundSolid(0xffffff_rgb),
    } << "Box";

    inflate(Centered::Expanding { box });

    (By::value(cornerLabel) | By::value(box))
        .check(areRightAligned(), "box and label are not right aligned")
        .check(areBottomAligned(), "box and label are not bottom aligned");
}

TEST_F(UILayoutTest, LayoutSpacing1) {
    inflate(Horizontal::Expanding {
      Button { "1" } with_style { Expanding {} },
    } with_style { LayoutSpacing { 8_dp }, FixedSize(200_dp, {}) });
    auto b = By::type<AButtonEx>().one();

    // checks the buttons margins are perfectly equal
    auto parent = b->getParent();
    EXPECT_EQ((parent->getSize() - b->getSize()) / 2, b->getPosition());
}
TEST_F(UILayoutTest, LayoutSpacing2) {
    inflate(Horizontal::Expanding {
      Button { "1" } with_style { Expanding {} },
      Button { "2" } with_style { Expanding {} },
    } with_style { LayoutSpacing { 8_dp }, FixedSize(200_dp, {}) });

    By::type<AButtonEx>().check(sameWidth(), "widths of the buttons are not equal");
}

TEST_F(UILayoutTest, LayoutSpacing3) {
    inflate(Horizontal::Expanding {
      Button { "1" } with_style { Expanding {} },
      Button { "2" } with_style { Expanding {} },
      Button { "3" } with_style { Expanding {} },
      Button { "4" } with_style { Expanding {} },
    } with_style { LayoutSpacing { 8_dp }, FixedSize(200_dp, {}) });

    By::type<AButtonEx>().check(sameWidth(), "widths of the buttons are not equal");
}

TEST_F(UILayoutTest, ExpandingWithMinSize1) {
    inflate(Horizontal::Expanding {
      _new<AView>() with_style { Expanding {}, MinSize { 200_dp, {} } } << ".expanding_minsize",
      _new<AView>() with_style { Expanding {} } << ".expanding",
    } with_style { FixedSize(300_dp, {}) });

    By::name(".expanding_minsize").check(width(200_dp), "width of .expanding_minsize is invalid");
    By::name(".expanding").check(width(100_dp), "width of .expanding is invalid");
}

TEST_F(UILayoutTest, ExpandingWithMinSize2) {
    inflate(Horizontal::Expanding {
      _new<AView>() with_style { Expanding {} } << ".expanding",
      _new<AView>() with_style { Expanding {}, MinSize { 200_dp, {} } } << ".expanding_minsize",
    } with_style { FixedSize(300_dp, {}) });

    By::name(".expanding_minsize").check(width(200_dp), "width of .expanding_minsize is invalid");
    By::name(".expanding").check(width(100_dp), "width of .expanding is invalid");
}

TEST_F(UILayoutTest, ExpandingWithMaxSize1) {
    inflate(Horizontal::Expanding {
        _new<AView>() with_style { Expanding {}, MaxSize { 100_dp, {} } } << ".expanding_maxsize",
        _new<AView>() with_style { Expanding {} } << ".expanding",
    } with_style { FixedSize(300_dp, {}) });

    By::name(".expanding_maxsize").check(width(100_dp), "width of .expanding_maxsize is invalid");
    By::name(".expanding").check(width(200_dp), "width of .expanding is invalid");
}

TEST_F(UILayoutTest, ExpandingWithMaxSize2) {
    inflate(Horizontal::Expanding {
        _new<AView>() with_style { Expanding {} } << ".expanding",
        _new<AView>() with_style { Expanding {}, MaxSize { 100_dp, {} } } << ".expanding_maxsize",
    } with_style { FixedSize(300_dp, {}) });

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
    l1->setText("test2");
    l1->getWindow()->applyGeometryToChildrenIfNecessary();

    EXPECT_GE(l2->getPositionInWindow().x, prevPosX);   // l2 is expected to shift to right.

    // extra layout update that should call LabelMock::getContentMinimumWidth one more time
    l1->getWindow()->applyGeometryToChildrenIfNecessary();
}
