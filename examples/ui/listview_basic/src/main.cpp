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

/// [AListView_example]
#include <AUI/Platform/Entry.h>
#include <AUI/Platform/AWindow.h>
#include <AUI/Util/UIBuildingHelpers.h>
#include "AUI/View/AListView.h"

using namespace ass;
using namespace declarative;

static AVector<AString> OPTIONS = {
    "New York", "London", "Tokyo", "Paris", "Berlin", "Moscow", "Sydney", "Dubai", "Singapore", "Toronto",
};

struct State {
    AProperty<std::size_t> selectedOption = 0;
};

_<AView> myDropdown(_<State> state) {
    return Vertical {
        ListView {
            .items = OPTIONS,
            .selectionId = AUI_REACT(state->selectedOption),
            .onSelectionChange = [state](std::size_t id) { state->selectedOption = id; },
        },
    };
}

AUI_ENTRY {
    auto window = _new<AWindow>("Dropdown list", 300_dp, 100_dp);
    auto state = _new<State>();
    window->setContents(
        Vertical {
            myDropdown(state),
            Label { AUI_REACT("Selected option: {}"_format(OPTIONS.at(state->selectedOption))) },
        }
    );
    window->show();
    return 0;
}
/// [AListView_example]