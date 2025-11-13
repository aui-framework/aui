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

/// [ASpacerExpanding_example]
#include "AUI/View/AButton.h"

#include <AUI/Platform/Entry.h>
#include <AUI/Platform/AWindow.h>
#include <AUI/Util/UIBuildingHelpers.h>
#include "AUI/View/ASpacerExpanding.h"

using namespace ass;
using namespace declarative;

AUI_ENTRY {
    auto window = _new<AWindow>("Spacer expanding", 300_dp, 100_dp);
    window->setContents(
        Vertical::Expanding {
            Button { Label { "Top" } },
            SpacerExpanding {},
            Button { Label { "Bottom" } },
        }
    );
    window->show();
    return 0;
}
/// [ASpacerExpanding_example]

