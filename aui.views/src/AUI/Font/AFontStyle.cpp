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

#include "AFontStyle.h"

#include "AUI/Platform/AFontManager.h"

size_t AFontStyle::getWidth(AStringView text) const
{
	return font->length(*this, text);
}

size_t AFontStyle::getWidth(std::u32string_view text) const
{
	return font->length(*this, text);
}

size_t AFontStyle::getLineHeight() const {
    return font->getAscenderHeight(size) * (1.f + lineSpacing);
}

// Normally, text will be rendered as this:
    //
    //
    //      - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    //      ^                          *@@@@@                                                 ^
    //      |                          #@@@@@                                                 |
    //    A |                          #@@@@@                                                 |
    //    S |                          #@@@@@                                                 |
    //    C |                          #@@@@@                                                 |
    //    E |        .:-=+******+=-.   #@@@@@     :+++++=                   .++++++.          |
    //    N |     .-*@@@@@@@@@@@@@@@#-.+@@@@@      +@@@@@+.                .#@@@@@-           |
    //    D |   .+@@@@@@%*-:...::=*%@@@%@@@@@       =@@@@@+.              .#@@@@@-            |
    //    E |  .%@@@@@#.            .#@@@@@@@        =@@@@@+              #@@@@@-             |
    //    R |  #@@@@@=.              .+@@@@@@        .-@@@@@+            *@@@@@-              | S
    //      | :@@@@@#.                .@@@@@@          -@@@@@+          +@@@@@-               | I
    //      | +@@@@@+                  #@@@@@           -@@@@@+        +@@@@@-                | Z
    //      | +@@@@@=                  *@@@@@            -@@@@@+.     =@@@@@=                 | E
    //      | -@@@@@*                  #@@@@@             :@@@@@+.   =@@@@@=                  |
    //      | .@@@@@@:                :@@@@@@              :@@@@@=  -@@@@@=                   |
    //      |  :@@@@@@-              :@@@@@@@               .@@@@@-.@@@@@=.                   |
    //      |   :#@@@@@%+-..   ...-+%@@@@@@@@                .%@@@%%@@@@=                     |
    //      |    .-*@@@@@@@@@@@@@@@@@*-:@@@@@                 .%@@@@@@@=                      |
    // -----+------ .:=**#%%%%%##*+-. - +#### ---------------- .%@@@@@= ----------------------------> baseline
    //    D |                                                 .=@@@@@=                        |
    //    E |                                                .+@@@@@=                         |
    //    S |                                               .%@@@@@-.                         |
    //    C |                                      --:::-=*%@@@@@*.                           |
    //    E |                                      %@@@@@@@@@@%+:.                            |
    //    N v                                      =**####*+-:.                               V
    //    D - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    //    E
    //    R
    //
    // However, it feels like it stuck to the top of the border. So, we'll just add the height of descender, so it
    // basically mirrors the descender at the bottom. This way, CAPITALS are centered, and text feels natural.
    //
    //      - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    //      ^
    //      |
    //      |   <-------- added height
    //      |
    //      |
    //      | - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    //      ^                          *@@@@@                                                 ^
    //      |                          #@@@@@                                                 |
    //    A |                          #@@@@@                                                 |
    //    S |                          #@@@@@                                                 |
    //    C |                          #@@@@@                                                 |
    //    E |        .:-=+******+=-.   #@@@@@     :+++++=                   .++++++.          |
    //    N |     .-*@@@@@@@@@@@@@@@#-.+@@@@@      +@@@@@+.                .#@@@@@-           |
    //    D |   .+@@@@@@%*-:...::=*%@@@%@@@@@       =@@@@@+.              .#@@@@@-            |
    //    E |  .%@@@@@#.            .#@@@@@@@        =@@@@@+              #@@@@@-             |
    //    R |  #@@@@@=.              .+@@@@@@        .-@@@@@+            *@@@@@-              | S
    //      | :@@@@@#.                .@@@@@@          -@@@@@+          +@@@@@-               | I
    //      | +@@@@@+                  #@@@@@           -@@@@@+        +@@@@@-                | Z
    //      | +@@@@@=                  *@@@@@            -@@@@@+.     =@@@@@=                 | E
    //      | -@@@@@*                  #@@@@@             :@@@@@+.   =@@@@@=                  |
    //      | .@@@@@@:                :@@@@@@              :@@@@@=  -@@@@@=                   |
    //      |  :@@@@@@-              :@@@@@@@               .@@@@@-.@@@@@=.                   |
    //      |   :#@@@@@%+-..   ...-+%@@@@@@@@                .%@@@%%@@@@=                     |
    //      |    .-*@@@@@@@@@@@@@@@@@*-:@@@@@                 .%@@@@@@@=                      |
    // -----+------ .:=**#%%%%%##*+-. - +#### ---------------- .%@@@@@= ----------------------------> baseline
    //    D |                                                 .=@@@@@=                        |
    //    E |                                                .+@@@@@=                         |
    //    S |                                               .%@@@@@-.                         |
    //    C |                                      --:::-=*%@@@@@*.                           |
    //    E |                                      %@@@@@@@@@@%+:.                            |
    //    N v                                      =**####*+-:.                               V
    //    D - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    //    E
    //    R

int AFontStyle::getTextBlockHeight() const {
    return getAscenderHeight() + 2 * getDescenderHeight();
}

int AFontStyle::getBaselineY(int top, int height, VerticalAlign verticalAlign) const {
    // adding height of descender, see the explanation above.
    int y = top + getAscenderHeight() + getDescenderHeight();

    if (verticalAlign == VerticalAlign::MIDDLE) {
        // the glyphs with the mirrored descender above them occupy getTextBlockHeight() pixels regardless of the line
        // spacing; center exactly this block within the area.
        y += (height - getTextBlockHeight()) / 2;
    }
    return y;
}
