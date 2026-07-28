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
#include "AUI/Util/ALayoutInflater.h"

namespace {

class View : public AView {
public:

protected:
    void commitStyle() override { AView::commitStyle(); }
};

using namespace declarative;

class UIStyleTest : public testing::UITest {
public:

protected:
    void SetUp() override {
        UITest::SetUp();

        mWindow = _new<AWindow>();
        ALayoutInflater::inflate(
            mWindow,
            Vertical {
              mView = _new<View>() AUI_OVERRIDE_STYLE { MinSize { 10_dp } },
              Label { "Some bullshit to complicate layout" },
            });
        mWindow->show();
    }

    void setupStateStyles() {
        mView AUI_OVERRIDE_STYLE {
            MinSize { 10_dp },
            BackgroundSolid { AColor::BLACK },

            on_state::Hovered {
              BackgroundSolid { AColor::RED },
            },

            on_state::Activated {
              BackgroundSolid { AColor::GREEN },
            },
        };
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

/**
 * Checks mouse move events.
 */
TEST_F(UIStyleTest, MouseMoveNoClick) {
    setupStateStyles();
    testing::InSequence s;

    By::type<View>().check(averageColor(AColor::BLACK));

    mWindow->onPointerMove({ 10, 10 }, {});   // somewhere over the mView
    By::type<View>().check(averageColor(AColor::RED));

    mWindow->onPointerMove({ 100, 100 }, {});   // somewhere outside the mView
    By::type<View>().check(averageColor(AColor::BLACK));
}

/**
 * Checks click with release outside the view.
 */
TEST_F(UIStyleTest, MouseMoveWithClick) {
    setupStateStyles();
    testing::InSequence s;

    By::type<View>().check(averageColor(AColor::BLACK));

    mWindow->onPointerMove({ 10, 10 }, {});   // somewhere over the mView
    By::type<View>().check(averageColor(AColor::RED));

    mWindow->onPointerPressed({ .position = { 10, 10 } });   // somewhere over the mView
    By::type<View>().check(averageColor(AColor::GREEN));

    mWindow->onPointerMove({ 100, 100 }, {});   // somewhere outside the mView
    By::type<View>().check(averageColor(AColor::GREEN));

    mWindow->onPointerReleased({ .position = { 100, 100 }, .triggerClick = false });   // somewhere outside the view
    By::type<View>().check(averageColor(AColor::BLACK));
}

/**
 * Checks click with release outside the view.
 */
TEST_F(UIStyleTest, Opacity) {
    setupStateStyles();
    testing::InSequence s;
    using namespace ass;

    mView AUI_OVERRIDE_STYLE {
        FixedSize(50_dp),
        BackgroundSolid(AColor::RED),
        Opacity(0),
    };
    By::type<View>().check(averageColor(AColor::WHITE), "view should disappear");
    mView AUI_OVERRIDE_STYLE {
        FixedSize(50_dp),
        BackgroundSolid(AColor::RED),
        Opacity(1),
    };
    By::type<View>().check(averageColor(AColor::RED), "view should appear");
}

TEST_F(UIStyleTest, AndSelector) {
    testing::InSequence s;
    using namespace ass;

    mWindow->setExtraStylesheet(AStylesheet { {
      ass::c(".one") && ass::c(".two"),
      BackgroundSolid { AColor::RED },
    } });

    mView->addAssName(".one");
    By::type<View>().check(averageColor(AColor::WHITE), "view should be uncolored");

    mView->addAssName(".two");
    By::type<View>().check(averageColor(AColor::RED), "view should be colored");
}

TEST_F(UIStyleTest, SetExtraStylesheet) {
    testing::InSequence s;
    using namespace ass;

    mView->addAssName(".one");
    By::type<View>().check(averageColor(AColor::WHITE), "view should be uncolored");

    mWindow->setExtraStylesheet(AStylesheet { {
      ass::c(".one"),
      BackgroundSolid { AColor::RED },
    } });
    By::type<View>().check(averageColor(AColor::RED), "view should be colored");

    mWindow->setExtraStylesheet(AStylesheet { {
      ass::c(".one"),
      BackgroundSolid { AColor::BLUE },
    } });
    By::type<View>().check(averageColor(AColor::BLUE), "view should be colored");
}

TEST_F(UIStyleTest, Merge) {
    testing::InSequence s;
    using namespace ass;

    mWindow->setExtraStylesheet(AStylesheet {
      {
        t<ASurface>(),
        BackgroundSolid { AColor::BLACK },
      },
      {
        ass::c("test1"),
        BackgroundImage { ":uni/caption/close.svg", {}, {}, Sizing::COVER }, // white cross
        FixedSize(32_px),
      },
      {
        ass::c("test2"),
        BackgroundImage { {}, AColor::RED },
      },
    });
    By::type<View>().check(pixelColorAt({0.5f, 0.5f}, AColor::BLACK)); // no white cross
    mView->addAssName("test1");
    By::type<View>().check(pixelColorAt({0.f, 0.5f}, AColor::BLACK)); // outside of cross
    By::type<View>().check(pixelColorAt({0.5f, 0.5f}, AColor::WHITE)); // within cross
    mView->addAssName("test2");
    By::type<View>().check(pixelColorAt({0.f, 0.5f}, AColor::BLACK)); // outside of cross
    By::type<View>().check(pixelColorAt({0.5f, 0.5f}, AColor::RED)); // within cross, overlayed by red
}
