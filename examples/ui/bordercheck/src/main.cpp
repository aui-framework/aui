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

#include <AUI/Platform/Entry.h>
#include <AUI/Platform/AWindow.h>
#include <AUI/Util/UIBuildingHelpers.h>

using namespace ass;
using namespace declarative;


AUI_ENTRY {
    auto window = _new<AWindow>("Checkbox", 300_dp, 100_dp);
    window->setContents(
        Vertical {
            Centered {
                _new<ALabel>("10%") AUI_OVERRIDE_STYLE {
                    BorderRadius { 10_pr },
                    FixedSize { 200_px , 40_px },
                    BackgroundSolid { 100_rgb },
                },

                _new<ALabel>("20%") AUI_OVERRIDE_STYLE {
                    Margin { 100_px, 0, 0, 0},
                    BorderRadius { 20_pr },
                    FixedSize { 200_px , 40_px },
                    BackgroundSolid { 150_rgb },
                },

                _new<ALabel>("50%") AUI_OVERRIDE_STYLE {
                    Margin { 200_px, 0, 0, 0},
                    BorderRadius { 50_pr },
                    FixedSize { 200_px , 40_px },
                    BackgroundSolid { 200_rgb },
                },
                _new<ALabel>("100%") AUI_OVERRIDE_STYLE {
                    Margin { 300_px, 0, 0, 0},
                    BorderRadius { 100_pr },
                    FixedSize { 200_px , 40_px },
                    BackgroundSolid { 300_rgb },
                },
                //Circle!
                _new<ALabel>("") AUI_OVERRIDE_STYLE {
                    Margin { 400_px, 0, 0, 0},
                    BorderRadius { 50_pr },
                    FixedSize { 40_px },
                    BackgroundSolid { 5148_rgb },
                }
            }
        }
    );
    window->show();
    return 0;
}
