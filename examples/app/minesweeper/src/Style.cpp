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

//
// Created by alex2772 on 1/4/21.
//

#include <AUI/View/AButton.h>
#include "CellView.h"
#include "MinesweeperWindow.h"
#include "NewGameWindow.h"
#include <AUI/ASS/ASS.h>

using namespace ass;

/// [CellSelector]
template<FieldCell fieldCell>
struct CellSelector: IAssSubSelector {
public:
    bool isPossiblyApplicable(AView* view) override {
        return dynamic_cast<CellView*>(view) != nullptr;
    }

    bool isStateApplicable(AView* view) override {
        if (auto c = dynamic_cast<CellView*>(view)) {
            return (c->fieldCell() & fieldCell) == fieldCell;
        }
        return false;
    }

    void setupConnections(AView* view, const _<AAssHelper>& helper) override {
        IAssSubSelector::setupConnections(view, helper);
        view->customCssPropertyChanged.clearAllOutgoingConnectionsWith(helper.get());
        AObject::connect(view->customCssPropertyChanged, AUI_SLOT(helper)::onInvalidateStateAss);
    }
};
/// [CellSelector]


/// [RevealSelector]
struct RevealSelector : IAssSubSelector {
public:
    bool isPossiblyApplicable(AView* view) override {
        return dynamic_cast<MinesweeperWindow*>(view) != nullptr;
    }

    bool isStateApplicable(AView* view) override {
        if (auto c = dynamic_cast<MinesweeperWindow*>(view)) {
            return c->isReveal();
        }
        return false;
    }

    void setupConnections(AView* view, const _<AAssHelper>& helper) override {
        IAssSubSelector::setupConnections(view, helper);
        view->customCssPropertyChanged.clearAllOutgoingConnectionsWith(helper.get());
        AObject::connect(view->customCssPropertyChanged, AUI_SLOT(helper)::onInvalidateStateAss);
    }
};
/// [RevealSelector]


struct GlobalStyle {
    GlobalStyle() {
        AStylesheet::global().addRules({
          {
            t<CellView>(),
            FixedSize { 26_dp },
            BackgroundSolid { 0xdedede_rgb },
            Border { 1_px, 0xeaeaea_rgb },
          },
          {
            !RevealSelector{} >> t<CellView>::hover(),
            BackgroundSolid { 0xfdfdfd_rgb },
          },
          /// [open]
          {
            CellSelector<FieldCell::OPEN>(),
            Border { 1_px, 0xffffff_rgb },
            BackgroundSolid { 0xeeeeee_rgb },
          },
          {
            CellSelector<FieldCell::HAS_FLAG>(),
            BackgroundImage { ":minesweeper/flag.svg" },
          },
          /// [open]

          // display mines for dead

          /// [reveal]
          {
            RevealSelector {} >> CellSelector<FieldCell::HAS_BOMB>(),
            BackgroundImage { ":minesweeper/bomb.svg" },
          },
          {
            RevealSelector {} >> CellSelector<FieldCell::HAS_FLAG>(),
            BackgroundImage { ":minesweeper/no_bomb_flag.svg" },
          },
          {
            RevealSelector {} >> CellSelector<FieldCell::HAS_FLAG | FieldCell::HAS_BOMB>(),
            BackgroundImage { ":minesweeper/bomb_flag.svg" },
          },
          /// [reveal]
          {
            CellSelector<FieldCell::RED_BG>(),
            BackgroundSolid { 0xff0000_rgb },
            Border { nullptr },
          },

          // misc
          {
            class_of(".frame"),
            Border { 1_dp, 0x444444_rgb },
          },
          { class_of(".frame") > t<AButton>(), Margin { 4_dp } },
          { t<NewGameWindow>(), Padding { 4_dp } },
        });
    }
} s;