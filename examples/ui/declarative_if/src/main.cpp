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

#include "AUI/View/Dynamic.h"

#include <AUI/Platform/Entry.h>
#include <AUI/Platform/AWindow.h>
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/ACheckBox.h>

using namespace ass;
using namespace declarative;

struct State {
    AProperty<bool> showSpoiler = false;
};

_<AView> minimalCheckBox(_<AProperty<bool>> state) {
    return CheckBox {
        .checked = AUI_REACT(*state),
        .onCheckedChange = [state](bool checked) { *state = checked; },
        .content = Label { "Show spoiler" },
    };
}

AUI_ENTRY {
    auto window = _new<AWindow>("Declarative if", 300_dp, 100_dp);
    auto state = _new<State>();
    window->setContents(
        Vertical {
            minimalCheckBox(AUI_PTR_ALIAS(state, showSpoiler)),
            experimental::Dynamic { AUI_REACT(state->showSpoiler ? Label { "UwU" } : _<AView>(nullptr)) }
        }
    );
    window->show();
    return 0;
}

