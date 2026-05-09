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
#include "AUI/View/AButton.h"
#include "AUI/View/AGroupBox.h"
#include "AUI/View/AListView.h"
#include "DemoListModel.h"

#include <AUI/Platform/Entry.h>
#include <AUI/Platform/AWindow.h>
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/ACheckBox.h>

using namespace ass;
using namespace declarative;

struct State {
    AProperty<bool> checked = false;
};

_<AView> minimalCheckBox(_<AProperty<bool>> state) {
    return CheckBox {
        .checked = AUI_REACT(*state),
        .onCheckedChange = [state](bool checked) { *state = checked; },
        .content = Label { "Minimal checkbox" },
    };
}

AUI_ENTRY {
    auto window = _new<AWindow>("Checkbox", 300_dp, 100_dp);
    auto state = _new<State>();
    window->setContents(
        Vertical {
            //minimalCheckBox(AUI_PTR_ALIAS(state, checked)),
            //Label { AUI_REACT(state->checked ? "Checkbox is checked" : "Checkbox is not checked") },

          // list view
                GroupBox {
                  Label { "List view" },
                  [] {   // lambda style inlining
                      auto model = _new<DemoListModel>();

                      return Vertical {
                          Horizontal {
                            _new<AButton>("Add").connect(&AButton::clicked, AUI_SLOT(model)::addItem),
                            _new<AButton>("Remove").connect(&AButton::clicked, AUI_SLOT(model)::removeItem),
                          } AUI_OVERRIDE_STYLE { LayoutSpacing { 4_dp } },
                          _new<AListView>(model)
                      } AUI_OVERRIDE_STYLE { LayoutSpacing { 4_dp } };
                  }(),
                },
        }
    );
    window->show();
    return 0;
}
/// [ACheckBox_example]

