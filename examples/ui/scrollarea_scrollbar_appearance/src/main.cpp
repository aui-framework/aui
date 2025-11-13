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

using namespace ass;
using namespace declarative;

_<AView> myScrollArea() {
    return ScrollArea {
        .verticalScrollbar = [](ScrollArea::ScrollbarInitParams params) {
            return Scrollbar {
                .direction = std::move(params.direction),
                .scroll = std::move(params.scroll),
                .viewportSize = std::move(params.viewportSize),
                .fullContentSize = std::move(params.fullContentSize),
                .scrollbarAppearance = ass::ScrollbarAppearance::ALWAYS, // <---
                .onScrollChange = std::move(params.onScrollChange),
            };
        },
        .content =
            Vertical {
              Label { "Long content" },
              Label { "Really long content" },
              Label { "This content keeps going" },
              Label { "And going" },
              Label { "And going some more" },
              Label { "Still not done" },
              Label { "Almost there" },
              Label { "Just kidding, more content" },
              Button { Label { "Button! lol wat" } },
              Label { "Getting longer" },
              Label { "No, it's long for real" },
            }
    };
}

AUI_ENTRY {
    auto window = _new<AWindow>("Scroll area", 300_dp, 100_dp);
    window->setContents(Vertical {
      myScrollArea(),
    });
    window->show();
    return 0;
}
/// [ScrollArea_example]