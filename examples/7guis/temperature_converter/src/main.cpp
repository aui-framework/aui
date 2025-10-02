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

#include <AUI/Platform/Entry.h>
#include <AUI/Platform/AWindow.h>
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/AButton.h>
#include "AUI/View/ATextField.h"
#include "AUI/View/ANumberPicker.h"

using namespace declarative;

auto myPicker() {
    return _new<ANumberPicker>() AUI_LET {
        it->setMin(-999);
        it->setMax(999);
    };
}

/// [window]
class TemperatureConverterWindow : public AWindow {
public:
    TemperatureConverterWindow() : AWindow("AUI - 7GUIs - TempConv", 300_dp, 50_dp) {
        setContents(Centered {
          Horizontal {
            myPicker() AUI_LET {
                biConnect(it->value(), mCelsius);
                it->focus();
            },
            Label { "°C" },
            Label { "=" } AUI_WITH_STYLE { Margin { {}, 16_dp } },
            myPicker() AUI_LET { biConnect(it->value(), mFahrenheit); },
            Label { "°F" },
          } AUI_WITH_STYLE { LayoutSpacing { 4_dp } },
        });

        connect(mFahrenheit.changed, [&] { mCelsius = (*mFahrenheit - 32.f) * (5.f / 9.f); });
        connect(mCelsius.changed, [&] { mFahrenheit = *mCelsius * (9.f / 5.f) + 32.f; });
    }

private:
    AProperty<int> mCelsius, mFahrenheit;
};
/// [window]

AUI_ENTRY {
    _new<TemperatureConverterWindow>()->show();
    return 0;
}