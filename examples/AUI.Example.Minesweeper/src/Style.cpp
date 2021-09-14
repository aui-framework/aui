/**
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
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
        AStylesheet::inst().addRules({
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