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
#include "AUI/View/AForEachUI.h"

#include <AUI/Platform/Entry.h>
#include <AUI/Platform/AWindow.h>
#include <AUI/Util/UIBuildingHelpers.h>
#include "AUI/View/AListView.h"

using namespace ass;
using namespace declarative;

static AVector<AString> OPTIONS = {
    "New York", "London", "Tokyo", "Paris", "Berlin", "Moscow", "Sydney", "Dubai", "Singapore", "Toronto",
};

namespace declarative {

namespace ListViewDefaults {
static _<AView> defaultBody(_<AView> content) {
    return Centered {
        std::move(content),
    } AUI_WITH_STYLE {
        BackgroundSolid { 0xffffff_rgb },
        Border { 1_dp, 0x828790_rgb },
        Padding { 2_dp },
        MinSize { 50_dp },
    };
}

static _<AView> defaultScrollArea(_<AView> content) {
    return ScrollArea { .content = std::move(content) };
}
}

template<typename T>
struct ListView {
    contract::In<AVector<T>> items;
    contract::In<std::size_t> selectionId;
    contract::Slot<std::size_t> onSelectionChange;

    std::function<_<AView>(_<AView>)> body = ListViewDefaults::defaultBody;
    std::function<_<AView>(_<AView>)> scrollArea = ListViewDefaults::defaultScrollArea;

    _<AView> operator()() {
        struct State {
            contract::In<AVector<T>> items;
        };
        auto state = aui::ptr::manage_shared(new State {
            .items = std::move(items),
        });

        return body(scrollArea(AUI_DECLARATIVE_FOR(i, state->items.value(), AVerticalLayout) {
            return Label { "{}"_format(i) };
        }));
    }
};
}

struct State {
    AProperty<std::size_t> selectedOption = 0;
};

_<AView> myListView(_<State> state) {
    return Vertical {
        ListView<AString> {
            .items = OPTIONS,
            .selectionId = AUI_REACT(state->selectedOption),
            .onSelectionChange = [state](std::size_t id) { state->selectedOption = id; },
        },
    };
}

AUI_ENTRY {
    auto window = _new<AWindow>("List view", 300_dp, 100_dp);
    auto state = _new<State>();
    window->setContents(
        Vertical {
            myListView(state),
            Label { AUI_REACT("Selected option: {}"_format(OPTIONS.at(state->selectedOption))) },
        }
    );
    window->show();
    return 0;
}
/// [AListView_example]