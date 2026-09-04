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
#include <AUI/Layout/AHorizontalLayout.h>
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/AButton.h>
#include <AUI/View/ALabel.h>
#include <AUI/View/ASpacerExpanding.h>
#include <AUI/View/AText.h>
#include "AUI/ASS/Property/Expanding.h"
#include "AUI/ASS/Property/FixedSize.h"
#include "AUI/ASS/Property/LayoutSpacing.h"
#include "AUI/Test/UI/By.h"
#include "AUI/Util/ALayoutInflater.h"
#include "AUI/View/AGroupBox.h"
#include "AUI/View/ASpacerFixed.h"
#include <cstdlib>

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

void settleLayout() {
    AUI_REPEAT(10) { uitest::frame(); }
}

void expectGeometry(const _<AView>& view, int x, int y, int width, int height) {
    EXPECT_EQ(view->getPosition().x, x);
    EXPECT_EQ(view->getPosition().y, y);
    EXPECT_EQ(view->getSize().x, width);
    EXPECT_EQ(view->getSize().y, height);
}
}   // namespace

// Checks for bug where cornerLabel goes outside of box.
TEST_F(UILayoutTest, SmallCorner1) {
    class View : public ALabel {
    public:
        using ALabel::ALabel;
        void onLayout(glm::ivec2 size) override {
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

    const int labelRight = cornerLabel->getPositionInWindow().x + cornerLabel->getWidth();
    const int labelBottom = cornerLabel->getPositionInWindow().y + cornerLabel->getHeight();
    const int boxRight = box->getPositionInWindow().x + box->getWidth();
    const int boxBottom = box->getPositionInWindow().y + box->getHeight();

    EXPECT_LE(std::abs(labelRight - boxRight), 1) << "box and label are not right aligned";
    EXPECT_LE(std::abs(labelBottom - boxBottom), 1) << "box and label are not bottom aligned";
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
    //EXPECT_LT(mWindow->getContentMinimumHeight(), i[0]->getMinimumHeight() + i[1]->getMinimumHeight());
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
        ON_CALL(*this, onComputeIntrinsicMinMaxAxis)
            .WillByDefault([this](int height) { return ALabel::onComputeIntrinsicMinMaxAxis(height); });
    }
    MOCK_METHOD(AMinMaxAxis, onComputeIntrinsicMinMaxAxis, (int height), (override));
};
}   // namespace

TEST_F(UILayoutTest, GetContentMinimumWidthPerformance1) {
    // checks how many times intrinsic size is computed.
    // in this test, it should call be exactly once.

    testing::InSequence s;
    auto l = _new<LabelMock>("test");
    EXPECT_CALL(*l, onComputeIntrinsicMinMaxAxis(testing::_)).Times(testing::AtMost(1));
    inflate(Centered { Horizontal {
      l,
    } });
    l->getWindow()->redraw();

    // extra layout update that should not call LabelMock::onComputeIntrinsicMinMaxAxis one more time
    AUI_REPEAT(10) { l->getWindow()->redraw(); }
}

TEST_F(UILayoutTest, GetContentMinimumWidthPerformance2) {
    // in contract to GetContentMinimumWidthPerformance1, here we additionally change text of the label, effectively
    // forcing two layout updates.

    testing::InSequence s;
    auto l1 = _new<LabelMock>("test");
    auto l2 = _new<ALabel>("test");
    EXPECT_CALL(*l1, onComputeIntrinsicMinMaxAxis(testing::_)).Times(testing::AtMost(2));
    inflate(Centered { Horizontal {
      l1,
      l2,
    } });
    l1->getWindow()->redraw();
    auto prevPosX = l2->getPositionInWindow().x;
    l1->text() = "test2";
    l1->getWindow()->redraw();

    EXPECT_GE(l2->getPositionInWindow().x, prevPosX);   // l2 is expected to shift to right.

    // extra layout update that should not compute intrinsic size one more time
    l1->getWindow()->redraw();
}

namespace {
}

TEST_F(UILayoutTest, Padding1) {
    auto mock = _new<AView>();
    mock->setExpanding();

    inflate(Centered {
        mock
    } AUI_OVERRIDE_STYLE { FixedSize(100_dp), Padding { 10_dp } });

    settleLayout();
    expectGeometry(mock, 10, 10, 80, 80);
}

TEST_F(UILayoutTest, Padding2) {
    auto mock = _new<AView>();
    mock->setExpanding();

    inflate(Centered {
        mock
    } AUI_OVERRIDE_STYLE { FixedSize(100_dp), Padding { 10_dp, 5_dp } });

    settleLayout();
    expectGeometry(mock, 5, 10, 90, 80);
}

TEST_F(UILayoutTest, Padding3) {
    auto mock = _new<AView>();
    mock->setExpanding();

    inflate(Centered { mock } AUI_OVERRIDE_STYLE { FixedSize(100_dp), Padding { 10_dp, 5_dp, 20_dp } });

    settleLayout();
    expectGeometry(mock, 5, 10, 90, 70);
}

TEST_F(UILayoutTest, Padding4) {
    auto mock = _new<AView>();
    mock->setExpanding();

    inflate(Centered { mock } AUI_OVERRIDE_STYLE { FixedSize(100_dp), Padding { 10_dp, 10_dp, 20_dp, 5_dp } });

    settleLayout();
    expectGeometry(mock, 5, 10, 85, 70);
}

TEST_F(UILayoutTest, Padding5) {
    auto mock = _new<AView>();
    mock->setExpanding();

    inflate(Horizontal {
        SpacerFixed { 5_dp },
        mock,
    } AUI_OVERRIDE_STYLE { FixedSize(100_dp), Padding { 10_dp, 10_dp, 20_dp, 5_dp } });

    settleLayout();
    expectGeometry(mock, 10, 10, 80, 70);
}

TEST_F(UILayoutTest, Padding6) {
    auto mock = _new<AView>();
    mock->setExpanding();

    inflate(Horizontal {
        mock,
        SpacerFixed { 5_dp },
    } AUI_OVERRIDE_STYLE { FixedSize(100_dp), Padding { 10_dp, 10_dp, 20_dp, 5_dp } });

    settleLayout();
    expectGeometry(mock, 5, 10, 80, 70);
}

TEST_F(UILayoutTest, Margin1) {
    auto mock = _new<AView>() AUI_OVERRIDE_STYLE {
        Expanding {},
        Margin { 10_dp },
    };

    inflate(Centered {
        mock
    } AUI_OVERRIDE_STYLE { FixedSize(100_dp) });

    settleLayout();
    expectGeometry(mock, 10, 10, 80, 80);
}

TEST_F(UILayoutTest, Margin2) {
    auto mock = _new<AView>() AUI_OVERRIDE_STYLE {
        Expanding {},
        Margin { 10_dp, 5_dp },
    };

    inflate(Centered { mock } AUI_OVERRIDE_STYLE { FixedSize(100_dp) });

    settleLayout();
    expectGeometry(mock, 5, 10, 90, 80);
}

TEST_F(UILayoutTest, Margin3) {
    auto mock = _new<AView>() AUI_OVERRIDE_STYLE {
        Expanding {},
        Margin { 10_dp, 5_dp, 20_dp },
    };

    inflate(Centered { mock } AUI_OVERRIDE_STYLE { FixedSize(100_dp) });

    settleLayout();
    expectGeometry(mock, 5, 10, 90, 70);
}

TEST_F(UILayoutTest, Margin4) {
    auto mock = _new<AView>() AUI_OVERRIDE_STYLE {
        Expanding {},
        Margin { 10_dp, 5_dp, 20_dp, 10_dp },
    };

    inflate(Centered { mock } AUI_OVERRIDE_STYLE { FixedSize(100_dp) });

    settleLayout();
    expectGeometry(mock, 10, 10, 85, 70);
}

TEST_F(UILayoutTest, Margin5) {
    auto mock = _new<AView>() AUI_OVERRIDE_STYLE {
        Expanding {},
        Margin { 10_dp, 10_dp, 20_dp, 5_dp },
    };

    inflate(
        Horizontal {
          SpacerFixed { 5_dp },
          mock,
        } AUI_OVERRIDE_STYLE { FixedSize(100_dp) });

    settleLayout();
    expectGeometry(mock, 10, 10, 80, 70);
}

TEST_F(UILayoutTest, Margin6) {
    auto mock = _new<AView>() AUI_OVERRIDE_STYLE {
        Expanding {},
        Margin { 10_dp, 10_dp, 20_dp, 5_dp },
    };

    inflate(
        Horizontal {
          mock,
          SpacerFixed { 5_dp },
        } AUI_OVERRIDE_STYLE { FixedSize(100_dp) });
    settleLayout();
    expectGeometry(mock, 5, 10, 80, 70);
}

TEST_F(UILayoutTest, Margin7) {
    auto mock = _new<AView>() AUI_OVERRIDE_STYLE {
        Expanding {},
        Margin { 10_dp, 10_dp, 20_dp, 5_dp },
    };

    inflate(
        Horizontal {
          SpacerFixed { 5_dp } AUI_OVERRIDE_STYLE { Margin { 5_dp } },
          mock,
        } AUI_OVERRIDE_STYLE { FixedSize(100_dp) });

    settleLayout();
    expectGeometry(mock, 20, 10, 70, 70);
}

TEST_F(UILayoutTest, Centered1) {
    auto mock = _new<AView>() AUI_OVERRIDE_STYLE {
        FixedSize { 4_dp },
    };

    // an FixedSize view in a Centered container => becomes in center (your captain).

    inflate(
        Centered {
          mock,
        } AUI_OVERRIDE_STYLE { FixedSize(100_dp) });

    settleLayout();
    expectGeometry(mock, 48, 48, 4, 4);
}

TEST_F(UILayoutTest, Centered2) {
    auto mock = _new<AView>() AUI_OVERRIDE_STYLE {
        MinSize { 4_dp },
    };

    // MinSize acts like FixedSize in a Centered.

    inflate(
        Centered {
          mock,
        } AUI_OVERRIDE_STYLE { FixedSize(100_dp) });

    settleLayout();
    expectGeometry(mock, 48, 48, 4, 4);
}

TEST_F(UILayoutTest, CenteredExpanding) {
    auto mock = _new<AView>() AUI_OVERRIDE_STYLE {
        Expanding {},
    };

    // an expanding in a Centered container => covers entire Centered.

    inflate(
        Centered {
          mock,
        } AUI_OVERRIDE_STYLE { FixedSize(100_dp) });

    settleLayout();
    expectGeometry(mock, 0, 0, 100, 100);
}

TEST_F(UILayoutTest, CenteredExpandingCappedByMax) {
    auto mock = _new<AView>() AUI_OVERRIDE_STYLE {
        Expanding {},
        MaxSize { 4_dp },
    };

    // MaxSize + Expanding acts like FixedSize in a Centered.

    inflate(
        Centered {
          mock,
        } AUI_OVERRIDE_STYLE { FixedSize(100_dp) });

    settleLayout();
    expectGeometry(mock, 48, 48, 4, 4);
}

TEST_F(UILayoutTest, CenteredOverflowingChildIsCenteredEqually) {
    // a child larger than its Centered container overflows it equally on all sides, and, since the container has a
    // fixed size, does not grow it.
    auto oversized = _new<AView>() AUI_OVERRIDE_STYLE {
        FixedSize { 200_dp },
    };
    _<AView> centered = Centered {
        oversized,
    } AUI_OVERRIDE_STYLE { FixedSize { 100_dp } };

    inflate(centered);

    settleLayout();
    EXPECT_EQ(*centered->size(), glm::ivec2(100));   // contents do not affect a fixed size.
    expectGeometry(oversized, -50, -50, 200, 200);   // (100 - 200) / 2 = -50

    const int overflowLeft = -oversized->getPosition().x;
    const int overflowRight = (oversized->getPosition().x + oversized->getSize().x) - centered->getSize().x;
    const int overflowTop = -oversized->getPosition().y;
    const int overflowBottom = (oversized->getPosition().y + oversized->getSize().y) - centered->getSize().y;
    EXPECT_EQ(overflowLeft, overflowRight) << "horizontal overflow is not equal on both sides";
    EXPECT_EQ(overflowTop, overflowBottom) << "vertical overflow is not equal on both sides";
}

// A container never gives its child more space than it has itself: a child whose minimum does not fit is squeezed
// down to the container instead of overflowing it.
TEST_F(UILayoutTest, MinSizeYieldsToCenteredContainer) {
    auto child = _new<AView>() AUI_OVERRIDE_STYLE {
        MinSize { 120_dp },
    };
    inflate(Centered { child } AUI_OVERRIDE_STYLE { FixedSize { 100_dp } });

    settleLayout();
    expectGeometry(child, 0, 0, 100, 100);
}

TEST_F(UILayoutTest, MinSizeYieldsToHorizontalContainer) {
    auto child = _new<AView>() AUI_OVERRIDE_STYLE {
        MinSize { 120_dp },
    };
    inflate(Horizontal { child } AUI_OVERRIDE_STYLE { FixedSize { 100_dp } });

    settleLayout();
    expectGeometry(child, 0, 0, 100, 100);
}

TEST_F(UILayoutTest, MinSizeYieldsToVerticalContainer) {
    auto child = _new<AView>() AUI_OVERRIDE_STYLE {
        MinSize { 120_dp },
    };
    inflate(Vertical { child } AUI_OVERRIDE_STYLE { FixedSize { 100_dp } });

    settleLayout();
    expectGeometry(child, 0, 0, 100, 100);
}

TEST_F(UILayoutTest, MinSizesAreSqueezedFromTheEndUntilTheyFit) {
    // 120 and 60 do not fit into 100: the trailing one gives up everything it has, and only the remaining deficit is
    // taken from the leading one.
    auto wide = _new<AView>() AUI_OVERRIDE_STYLE { MinSize { 120_dp, {} } };
    auto narrow = _new<AView>() AUI_OVERRIDE_STYLE { MinSize { 60_dp, {} } };
    inflate(Horizontal {
        wide,
        narrow,
    } AUI_OVERRIDE_STYLE { FixedSize { 100_dp } });

    settleLayout();
    EXPECT_EQ(wide->getPosition().x, 0);
    EXPECT_EQ(wide->getSize().x, 100);
    EXPECT_EQ(narrow->getSize().x, 0);
    EXPECT_EQ(wide->getSize().x + narrow->getSize().x, 100) << "children do not fill the container exactly";
}

TEST_F(UILayoutTest, FixedSizeChildIsNotSqueezed) {
    // ...but a fixed size is law: such a child keeps its size and overflows the container instead.
    auto child = _new<AView>() AUI_OVERRIDE_STYLE {
        FixedSize { 200_dp },
    };
    inflate(Horizontal { child } AUI_OVERRIDE_STYLE { FixedSize { 100_dp } });

    settleLayout();
    EXPECT_EQ(child->getSize().x, 200);
    EXPECT_EQ(child->getSize().y, 200);
}

// When the children do not fit, the ones that come first keep their size: the deficit is taken from the end, so a
// toolbar loses its trailing items rather than squeezing everything at once.
TEST_F(UILayoutTest, OvercommitmentKeepsEarlierChildrenIntact) {
    auto first = _new<AView>() AUI_OVERRIDE_STYLE { MinSize { 60_dp, {} } };
    auto second = _new<AView>() AUI_OVERRIDE_STYLE { MinSize { 60_dp, {} } };

    inflate(Horizontal {
        first,
        second,
    } AUI_OVERRIDE_STYLE { FixedSize { 100_dp } });

    settleLayout();
    EXPECT_EQ(first->getSize().x, 60) << "the first child shrank while the one after it still had space to give";
    EXPECT_EQ(second->getPosition().x, 60);
    EXPECT_EQ(second->getSize().x, 40);
}

// examples/ui/overcommitment_horizontal: two buttons in a row too narrow for both of them. The leading one stays
// fully visible; it is the trailing one that runs out of room.
TEST_F(UILayoutTest, OvercommittedRowKeepsLeadingButtonAtItsNaturalWidth) {
    _<AView> visible = Button { Label { "Visible" } };
    _<AView> overlapping = Button { Label { "Overlaps with border" } };

    inflate(Vertical {
        Horizontal {
            visible,
            overlapping,
        },
        Button { Label { "OK" } },
    } AUI_OVERRIDE_STYLE { FixedSize { 100_dp } });

    settleLayout();
    EXPECT_GT(visible->getSize().x, 0);
    EXPECT_EQ(visible->getSize().x, visible->measure(AConstraints {}).x)
        << "the leading button did not keep its natural width";
    EXPECT_LT(overlapping->getSize().x, overlapping->measure(AConstraints {}).x)
        << "the trailing button was expected to be the one running out of room";
}

TEST_F(UILayoutTest, OvercommitmentIsTakenFromTheEndBackwards) {
    auto first = _new<AView>() AUI_OVERRIDE_STYLE { MinSize { 60_dp, {} } };
    auto second = _new<AView>() AUI_OVERRIDE_STYLE { MinSize { 60_dp, {} } };
    auto third = _new<AView>() AUI_OVERRIDE_STYLE { MinSize { 60_dp, {} } };

    inflate(Horizontal {
        first,
        second,
        third,
    } AUI_OVERRIDE_STYLE { FixedSize { 100_dp } });

    settleLayout();
    // the last one gives everything it has, and only then the one before it gives the rest.
    EXPECT_EQ(first->getSize().x, 60);
    EXPECT_EQ(second->getSize().x, 40);
    EXPECT_EQ(third->getSize().x, 0);
}

// Identical group boxes stacked in a Vertical must all fit their contents: none of them may collapse, and none may
// hog the container.
TEST_F(UILayoutTest, GroupBoxesInVerticalKeepTheirContentHeight) {
    struct Box {
        _<AView> box, title, body;
    };
    AVector<Box> boxes;
    auto groupBox = [&] {
        _<AView> title = Label { "Title" };
        _<AView> body = Label { "Body" };
        _<AView> box = GroupBox {
            title,
            Vertical {
                body,
            },
        };
        boxes << Box { box, title, body };
        return box;
    };

    inflate(Vertical {
        groupBox(),
        groupBox(),
        groupBox(),
        groupBox(),
    } AUI_OVERRIDE_STYLE { FixedSize { 200_dp, 400_dp } });

    settleLayout();
    for (const auto& i : boxes) {
        EXPECT_GT(i.title->getSize().y, 0) << "title collapsed";
        EXPECT_GT(i.body->getSize().y, 0) << "body collapsed";
        EXPECT_GE(i.box->getSize().y, i.title->getSize().y + i.body->getSize().y)
            << "group box does not fit its own contents";
        EXPECT_EQ(i.box->getSize().y, boxes.first().box->getSize().y)
            << "group boxes with equal contents differ in height";
    }
}

TEST_F(UILayoutTest, FixedSizeIsLaw) {
    // a FixedSize view is exactly that size, no matter how large its contents are.
    auto oversizedContent = _new<AView>() AUI_OVERRIDE_STYLE {
        FixedSize { 100_dp },
    };
    auto box = Horizontal {
        oversizedContent,
    } AUI_OVERRIDE_STYLE { FixedSize { 52_dp } };

    inflate(Centered { box } AUI_OVERRIDE_STYLE { FixedSize(200_dp) });

    settleLayout();
    EXPECT_EQ(box->measure(AConstraints {}), glm::ivec2(52));
    expectGeometry(box, 74, 74, 52, 52);   // (200 - 52) / 2 = 74
}

TEST_F(UILayoutTest, ExpandingTextHasNonZeroIntrinsicWidth) {
    auto text = AText::fromString("middle");
    EXPECT_GT(text->computeMinMaxAxis().max, 0);
}

TEST_F(UILayoutTest, HorizontalSpacerExpandingConsumesRemainingSpace) {
    auto leading = _new<AView>() AUI_OVERRIDE_STYLE { FixedSize { 24_dp } };
    auto spacer = _new<ASpacerExpanding>();
    auto trailing = _new<AView>() AUI_OVERRIDE_STYLE { FixedSize { 24_dp } };

    inflate(Horizontal {
        leading,
        spacer,
        trailing,
    } AUI_OVERRIDE_STYLE { FixedSize { 100_dp, 24_dp } });

    settleLayout();
    expectGeometry(leading, 0, 0, 24, 24);
    expectGeometry(spacer, 24, 0, 52, 24);
    expectGeometry(trailing, 76, 0, 24, 24);
}

TEST_F(UILayoutTest, HorizontalFixedExpandingTextFixedKeepsTrailingChildInside) {
    auto leading = _new<AView>() AUI_OVERRIDE_STYLE { FixedSize { 24_dp } };
    auto text = AText::fromString("middle");
    auto trailing = _new<AView>() AUI_OVERRIDE_STYLE { FixedSize { 24_dp } };

    inflate(Horizontal {
        leading,
        text,
        trailing,
    } AUI_OVERRIDE_STYLE { FixedSize { 120_dp, 24_dp } });

    settleLayout();
    expectGeometry(leading, 0, 0, 24, 24);
    EXPECT_EQ(text->getPosition().x, 24);
    EXPECT_EQ(text->getSize().x, 72);
    EXPECT_EQ(trailing->getPosition().x, 96);
    EXPECT_EQ(trailing->getSize().x, 24);
}

TEST_F(UILayoutTest, ExpandingDoesNotAffectParent1) {
    // this test verifies that an Expanding view (i.e. a spacer) does not affect parents (i.e., wrapper) geometry.
    // in AUI, spacing affects the environment **within** a container, not the container itself.
    _<AView> spacer = SpacerExpanding {};
    _<AView> wrapper = Vertical { spacer };
    inflate(
        Stacked {
          wrapper,
        } AUI_OVERRIDE_STYLE { FixedSize { 100_dp, 100_dp } });

    settleLayout();
    EXPECT_EQ(*wrapper->size(), glm::ivec2(0)); // no content -> zero size.
}

TEST_F(UILayoutTest, ExpandingDoesNotAffectParent2) {
    // same as ExpandingDoesNotAffectParent1 but using Horizontal instead.
    _<AView> spacer = SpacerExpanding {};
    _<AView> wrapper = Horizontal { spacer };
    inflate(
        Stacked {
          wrapper,
        } AUI_OVERRIDE_STYLE { FixedSize { 100_dp, 100_dp } });

    settleLayout();
    EXPECT_EQ(*wrapper->size(), glm::ivec2(0)); // no content -> zero size.
}
