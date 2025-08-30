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

/// [AButton_example]
#include <AUI/Platform/Entry.h>
#include <AUI/Platform/AWindow.h>
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/AButton.h>

using namespace ass;
using namespace declarative;

AUI_ENTRY {
    auto window = _new<AWindow>("Button", 600_dp, 300_dp);
    window->setContents(Centered {
      Button {
        .content = "Click me",
        .onClick = [] { ALogger::info("Test") << "Hello world!"; },
      },
    });
    window->show();
    return 0;
}
/// [AButton_example]

