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

using namespace declarative;

struct State {
    AProperty<bool> checked = false;
};

_<AView> minimalCheckBox(contract::In<bool> checked, std::function<void(bool)> onCheckedChange) {
    return CheckBox {
        .checked = std::move(checked),
        .onCheckedChange = std::move(onCheckedChange),
        .content = Label { "Green" },
    };
}

AUI_ENTRY {
    auto window = _new<AWindow>("Checkbox", 300_dp, 100_dp);
    auto state = _new<State>();
    window->setContents(Vertical {
      minimalCheckBox(AUI_REACT(state->checked), [=](bool checked) { state->checked = checked; }),
      Label {
        .text = "This label reflects checkbox state.",
        .modifier = AUI_REACT(Modifier {} | TextColor { state->checked ? AColor::GREEN : AColor::RED }),
      },
    });
    window->show();
    return 0;
}



/// [ACheckBox_example]
