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
#include "AUI/View/AColorPicker.h"
#include "AUI/View/AGroupBox.h"

#include <AUI/Platform/Entry.h>
#include <AUI/Platform/AWindow.h>
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/ACheckBox.h>

using namespace declarative;

struct State {
    AProperty<AColor> colorTop = AColor::RED;
    AProperty<AColor> colorBottom = AColor::BLUE;
};

namespace declarative {

struct View {
    contract::In<ass::Modifier> modifier;

    _<AView> operator()() {
        auto view = _new<AView>();
        modifier.bindTo(AUI_SLOT(view.get())::setModifier);
        return view;
    }
};
}


struct TitledColorPicker {
    contract::In<AString> title = "Color picker";
    contract::In<AColor> color;
    std::function<void(AColor)> onColorChanged;

    _<AView> operator()() {
        return GroupBox {
            Horizontal { color_picker::ColorView(color), Label { "Top color" } },
            color_picker::SlidersRGB {
              .color = color,
              .onColorChanged = std::move(onColorChanged),
            },
        };
    }
};

AUI_ENTRY {
    auto window = _new<AWindow>("Gradient Builder", 300_dp, 100_dp);
    auto state = _new<State>();
    window->setContents(Horizontal {
        View {
            .modifier = AUI_REACT(Modifier {}
                | BackgroundGradient(state->colorTop, state->colorBottom, 180_deg)
                | MinSize { 100_dp, 100_dp }),
        },
        Vertical {
            TitledColorPicker {
                .title = "Top",
                .color = AUI_REACT(state->colorTop),
                .onColorChanged = [state](AColor color) { state->colorTop = color; },
            },
            TitledColorPicker {
                .title = "Bottom",
                .color = AUI_REACT(state->colorBottom),
                .onColorChanged = [state](AColor color) { state->colorBottom = color; },
            },
        },
    });
    window->show();
    return 0;
}

/// [ACheckBox_example]
