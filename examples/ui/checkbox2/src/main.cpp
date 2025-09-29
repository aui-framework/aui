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
    AProperty<bool> option1 = false;
    AProperty<bool> option2 = false;
    AProperty<bool> option3 = false;

    bool isAllOptionsSelected() const {
        return option1 && option2 && option3;
    }
};

_<AView> parentCheckBox(_<State> state) {
    return Vertical {
        CheckBox {
            .checked = AUI_REACT(state->isAllOptionsSelected()),
            .onCheckedChange = [state](bool checked) {
              state->option1 = checked;
              state->option2 = checked;
              state->option3 = checked;
            },
            .content = Label { "Select all" },
        },
        CheckBox {
            .checked = AUI_REACT(state->option1),
            .onCheckedChange = [state](bool checked) { state->option1 = checked; },
            .content = Label { "Option 1" },
        },
        CheckBox {
            .checked = AUI_REACT(state->option2),
            .onCheckedChange = [state](bool checked) { state->option2 = checked; },
            .content = Label { "Option 2" },
        },
        CheckBox {
            .checked = AUI_REACT(state->option3),
            .onCheckedChange = [state](bool checked) { state->option3 = checked; },
            .content = Label { "Option 3" },
        },
    };
}

AUI_ENTRY {
    auto window = _new<AWindow>("Checkbox", 300_dp, 100_dp);
    auto state = _new<State>();
    window->setContents(
        Vertical {
            parentCheckBox(state),
            Label { AUI_REACT(state->isAllOptionsSelected() ? "All options selected" : "Please select all options") },
        }
    );
    window->show();
    return 0;
}
/// [ACheckBox_example]

