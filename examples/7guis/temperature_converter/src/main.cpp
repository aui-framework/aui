/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
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
    return _new<ANumberPicker>() let {
        it->setMin(-999);
        it->setMax(999);
    };
}

class TemperatureConverterWindow : public AWindow {
public:
    TemperatureConverterWindow() : AWindow("AUI - 7GUIs - TempConv", 300_dp, 50_dp) {
        setContents(Centered {
          Horizontal {
            myPicker() let {
                biConnect(it->value(), mCelsius);
                it->focus();
            },
            Label { "°C" },
            Label { "=" } with_style { Margin { {}, 16_dp } },
            myPicker() let { biConnect(it->value(), mFahrenheit); },
            Label { "°F" },
          },
        });

        connect(mFahrenheit.changed, [&] { mCelsius = (*mFahrenheit - 32.f) * (5.f / 9.f); });
        connect(mCelsius.changed, [&] { mFahrenheit = *mCelsius * (9.f / 5.f) + 32.f; });
    }

private:
    AProperty<int> mCelsius, mFahrenheit;
};

AUI_ENTRY {
    _new<TemperatureConverterWindow>()->show();
    return 0;
}