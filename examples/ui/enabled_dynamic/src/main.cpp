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

/// [AButton_enabled_example]
#include "AUI/Platform/AMessageBox.h"

#include <AUI/Platform/Entry.h>
#include <AUI/Platform/AWindow.h>
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/AButton.h>
#include <AUI/View/ACheckBox.h>
#include <AUI/ASS/Property/Enabled.h>

using namespace declarative;

struct State {
    AProperty<bool> buttonEnabled = true;
};

AUI_ENTRY {
    auto window = _new<AWindow>("Dynamic Button Enable/Disable", 400_dp, 150_dp);
    auto state = _new<State>();

    window->setContents(Vertical {
      CheckBox {
        .checked = AUI_REACT(state->buttonEnabled),
        .onCheckedChange = [state](bool checked) { state->buttonEnabled = checked; },
        .content = Label { "Toggle button state" },
      },
      Label {
        .text = "This button's enabled state is controlled by Enabled modifier.",
      },
      Button {
        .content = Label { "This button does nothing" },
        .onClick =
            [] { AMessageBox::show(dynamic_cast<AWindow*>(AWindow::current()), "Dynamic", "No, it actually does"); },
        .modifier = AUI_REACT(Modifier {} | Enabled { state->buttonEnabled }),
      },
    });
    window->show();
    return 0;
}

/// [AButton_enabled_example]
