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

/// [ACheckBox_example]
#include <AUI/Platform/Entry.h>
#include <AUI/Platform/AWindow.h>
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/ACheckBox.h>

using namespace ass;
using namespace declarative;

struct State {
    AProperty<bool> checked = false;
};

_<AView> minimalCheckBox(_<State> state) {
    return CheckBox {
        .checked = AUI_REACT(state->checked),
        .onCheckedChange = [state](bool checked) { state->checked = checked; },
        .content = Label { "Minimal checkbox" },
    };
}

AUI_ENTRY {
    auto window = _new<AWindow>("Checkbox", 300_dp, 100_dp);
    auto state = _new<State>();
    window->setContents(
        Vertical {
            minimalCheckBox(state),
            Label { AUI_REACT(state->checked ? "Checkbox is checked" : "Checkbox is not checked") },
        }
    );
    window->show();
    return 0;
}
/// [ACheckBox_example]

