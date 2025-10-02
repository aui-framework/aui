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
#include "AUI/View/AProgressBar.h"

using namespace ass;
using namespace declarative;

struct State {
    AProperty<aui::float_within_0_1> progress = 0.42f;
};

_<AView> minimalProgress(_<State> state) {
    return Vertical {
        ProgressBar {
            .progress = AUI_REACT(state->progress),
        },
    };
}

AUI_ENTRY {
    auto window = _new<AWindow>("Checkbox", 300_dp, 100_dp);
    auto state = _new<State>();
    window->setContents(
        Vertical {
          minimalProgress(state),
          Label { AUI_REACT("Progress: {}%"_format(int(state->progress * 100))) },
        }
    );
    window->show();
    return 0;
}
/// [ACheckBox_example]

