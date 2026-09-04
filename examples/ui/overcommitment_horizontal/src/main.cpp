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

#include <range/v3/all.hpp>
#include <AUI/View/AForEachUI.h>
#include <AUI/Platform/Entry.h>
#include "AUI/Platform/AWindow.h"
#include "AUI/Util/UIBuildingHelpers.h"
#include "AUI/View/AScrollArea.h"
#include "AUI/Model/AListModel.h"
#include <AUI/View/ATextField.h>
#include <AUI/View/AText.h>

#include <AUI/View/ASpacerFixed.h>
#include "AUI/Platform/AMessageBox.h"
#include "AUI/View/AButton.h"
#include "AUI/View/Dynamic.h"

using namespace declarative;
using namespace ass;
using namespace std::chrono_literals;

static AArc<AView> root() {
    return Vertical {
        Horizontal {
            Button { .content = Label { "Visible" } },
            Button { .content = Label { "Overlaps with border" } },
        } AUI_OVERRIDE_STYLE { Border { 1_px, AColor::RED } },
        Button { .content = Label { "OK" } },
    } AUI_OVERRIDE_STYLE { FixedSize { 100_dp }, Border { 1_px, AColor::GRAY } };
}

AUI_ENTRY {
    auto window = _new<AWindow>();
    window->setContents(Centered { root() });
    window->show();
    return 0;
}
