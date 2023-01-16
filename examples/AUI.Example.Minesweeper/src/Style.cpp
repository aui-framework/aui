// AUI Framework - Declarative UI toolkit for modern C++17
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