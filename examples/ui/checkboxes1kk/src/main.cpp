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
#include <range/v3/all.hpp>
#include "AUI/View/AButton.h"
#include "AUI/View/ACheckBox.h"
#include "AUI/View/AForEachUI.h"

#include <AUI/Platform/Entry.h>
#include <AUI/Platform/AWindow.h>
#include <AUI/Util/UIBuildingHelpers.h>
#include "AUI/View/AScrollArea.h"
#include "AUI/View/ATextArea.h"

using namespace ass;
using namespace declarative;

static constexpr auto COUNT = 1'000'000;
static constexpr auto DIV = 1'000;
static const auto INDICES = ranges::views::ints | ranges::view::take(COUNT);

AUI_ENTRY {
    auto window = _new<AWindow>("1000000 checkboxes", 300_dp, 300_dp);
    struct State {
        AProperty<std::vector<bool>> values = std::vector<bool>(COUNT);
        ScrollAreaViewport::State scrollAreaState;
        APropertyPrecomputed<size_t> observerCount = [&] {
            aui::react::DependencyObserverScope::addDependency(scrollAreaState.scroll.changed);
            return values.changed.outgoingConnections().size();
        };
    };
    auto state = _new<State>();
    window->setContents(Vertical {
      ScrollArea {
        .state = AUI_PTR_ALIAS(state, scrollAreaState),
        .content =
            AUI_DECLARATIVE_FOR(row, INDICES | ranges::view::chunk(DIV), AVerticalLayout) {
                return Horizontal {
                    Label { "{}"_format(ranges::front(row) / DIV) } AUI_WITH_STYLE { FixedSize { 50_dp, {} } },
                    // AUI_DECLARATIVE_FOR(i, row, AHorizontalLayout) {
                    //     return CheckBox {
                    //         .checked = AUI_REACT(state->values->at(i)),
                    //         .onCheckedChange = [state, i](bool v) { state->values.writeScope()->at(i) = v; },
                    //     } AUI_WITH_STYLE { Margin { 2_dp } };
                    // } AUI_LET { it->setKeyFunction([](int i) { return i; }); },
                };
            } AUI_LET { it->setKeyFunction([](const auto& row) { return ranges::front(row); }); },
      },
      Label { AUI_REACT("Observers: {}"_format(state->observerCount)) },
    });
    window->show();
    return 0;
}
/// [ScrollArea_example]