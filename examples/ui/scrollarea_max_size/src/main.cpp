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

/// [ScrollArea_example]
#include "AUI/View/AButton.h"
#include "AUI/View/AForEachUI.h"

#include <AUI/Platform/Entry.h>
#include <AUI/Platform/AWindow.h>
#include <AUI/Util/UIBuildingHelpers.h>
#include "AUI/View/AScrollArea.h"
#include "AUI/View/ATextArea.h"

using namespace ass;
using namespace declarative;

_<AView> myScrollArea() {
    return ScrollArea {
        .content =
            _new<ATextArea>() AUI_WITH_STYLE {
              MinSize { 50_dp },
            },
    } AUI_WITH_STYLE {
        Expanding { 1, 0 }, // occupy all width
        MaxSize { {}, 200_dp },     // restrict height
    };
}

_<AView> whiteBox(_<AView> view) {
    return Stacked { std::move(view) } AUI_WITH_STYLE {
        BackgroundSolid { AColor::WHITE },
        Padding { 2_px },
        Border { 1_px, AColor::GRAY },
    };
}

AUI_ENTRY {
    auto window = _new<AWindow>("Scroll area", 300_dp, 300_dp);
    window->setContents(Vertical {
      Label { "Contents before" },
      whiteBox(myScrollArea()),
      Label { "Contents after" },
    });
    window->show();
    return 0;
}
/// [ScrollArea_example]