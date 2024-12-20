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

//
// Created by alex2772 on 1/4/21.
//


#include <AUI/View/AButton.h>
#include "CellView.h"
#include "MinesweeperWindow.h"
#include "NewGameWindow.h"
#include <AUI/ASS/ASS.h>

using namespace ass;

struct style {
    style() {
        AStylesheet::global().addRules({
            {
                any<CellView>(),
                FixedSize { 26_dp },
                BackgroundSolid { 0xdedede_rgb },
                Border {1_px, 0xeaeaea_rgb },
            },
            {
                any<CellView>::hover(),
                BackgroundSolid { 0xfdfdfd_rgb },
            },
            {
                any<CellView>()["open"],
                Border {1_px, 0xffffff_rgb },
                BackgroundSolid { 0xeeeeee_rgb },
            },
            {
                any<CellView>()["flag"],
                BackgroundImage { ":minesweeper/flag.svg" }
            },

            // display mines for dead
            {
                any<MinesweeperWindow>()["dead"] >> any<CellView>()["bomb"],
                BackgroundImage { ":minesweeper/bomb.svg" }
            },
            {
                any<MinesweeperWindow>()["dead"] >> any<CellView>()["flag"],
                BackgroundImage { ":minesweeper/no_bomb_flag.svg" }
            },
            {
                any<MinesweeperWindow>()["dead"] >> any<CellView>()["bomb"]["flag"],
                BackgroundImage { ":minesweeper/bomb_flag.svg" }
            },
            {
                any<CellView>()["red"],
                BackgroundSolid { 0xff0000_rgb },
                Border { nullptr },
            },

            // misc
            {
                class_of(".frame"),
                Border { 1_dp, 0x444444_rgb },
            },
            {
                class_of(".frame") > any<AButton>(),
                Margin { 4_dp }
            },
            {
                any<NewGameWindow>(),
                Padding { 4_dp }
            }
        });
    }
} s;