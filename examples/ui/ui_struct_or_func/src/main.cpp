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

#include "AUI/View/AButton.h"

#include <AUI/Platform/Entry.h>
#include <AUI/Platform/AWindow.h>
#include <AUI/Util/UIBuildingHelpers.h>
#include "AUI/View/AProgressBar.h"
#include "AUI/View/ASlider.h"

using namespace ass;
using namespace declarative;

struct State {
    AProperty<float> degress = 0;   // 0-360
};

// This example shows two ways of defining custom views: through function or through struct

_<AView> sliderMinMax(contract::In<float> value, std::function<void(float)> onValueChanged, contract::In<float> min = 0.f, contract::In<float> max = 100.f) {
    return Slider {
        .value = AUI_REACT((*value - *min) / (*max - *min)),
        .onValueChanged = [onValueChanged = std::move(onValueChanged), min, max](float value) {
            onValueChanged(glm::mix(*min, *max, value));
        }
    };
}

struct SliderMinMax {
    contract::In<float> value;
    std::function<void(float)> onValueChanged;
    contract::In<float> min = 0.f;
    contract::In<float> max = 100.f;

    _<AView> operator()() {
        auto value = std::move(this->value);
        auto min = std::move(this->min);
        auto max = std::move(this->max);
        return Slider {
            .value = AUI_REACT((*value - *min) / (*max - *min)),
            .onValueChanged = [onValueChanged = std::move(onValueChanged), min, max](float value) {
                onValueChanged(glm::mix(*min, *max, value));
            }
        };
    }
};

AUI_ENTRY {
    auto window = _new<AWindow>("ui_struct_or_func", 300_dp, 100_dp);
    auto state = _new<State>();
    window->setContents(Vertical {
      sliderMinMax(AUI_REACT(state->degress), [state](float v){ state->degress = v; }, 0.f, 360.f),
      SliderMinMax {
          .value = AUI_REACT(state->degress),
          .onValueChanged = [state](float v){ state->degress = v; },
          .min = 0.f,
          .max = 360.f,
      },
      Label { AUI_REACT("Value: {}°"_format(int(state->degress))) },
    });
    window->show();
    return 0;
}
