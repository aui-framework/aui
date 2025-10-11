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

/// [ARadioButton_example]
#include <AUI/Platform/Entry.h>
#include <AUI/Platform/AWindow.h>
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/AForEachUI.h>
#include "AUI/View/ARadioButton.h"

using namespace ass;
using namespace declarative;

struct State {
    AProperty<int> selection = 0;
};

_<AView> radioButtons(_<State> state) {
    static constexpr auto options = std::array {
        std::make_tuple(0, "Option 0"),
        std::make_tuple(1, "Option 1"),
        std::make_tuple(2, "Option 2"),
    };

    return AUI_DECLARATIVE_FOR(i, options, AVerticalLayout) {
        const auto& [index, text] = i;
        return RadioButton {
            .checked = AUI_REACT(state->selection == index),
            .onClick = [state, index] { state->selection = index; },
            .content = Label { text },
        };
    };
}

AUI_ENTRY {
    auto window = _new<AWindow>("Radiobutton", 600_dp, 300_dp);
    auto state = _new<State>();
    window->setContents(
      Vertical {
        radioButtons(state),
        Label { AUI_REACT("Selected option: {}"_format(state->selection)) },
      }
    );
    window->show();
    return 0;
}
/// [ARadioButton_example]

