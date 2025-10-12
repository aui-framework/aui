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

using namespace declarative;

/// [counter]
class CounterWindow : public AWindow {
public:
    CounterWindow() : AWindow("AUI - 7GUIs - Counter", 200_dp, 100_dp) {
        setContents(Centered {
          Horizontal {
            Label { AUI_REACT("Count: {}"_format(mCounter)) },
            Button { Label { "Count" }, [this] { mCounter += 1; } },
          } AUI_WITH_STYLE { LayoutSpacing { 4_dp } },
        });
    }

private:
    AProperty<int> mCounter;
};
/// [counter]

AUI_ENTRY {
    _new<CounterWindow>()->show();
    return 0;
}