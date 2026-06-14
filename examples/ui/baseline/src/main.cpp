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
#include <AUI/View/AGroupBox.h>
#include "AUI/View/AText.h"
#include "AUI/View/ATextField.h"
#include "AUI/View/ATextArea.h"
#include "AUI/View/ADrawableView.h"
#include "AUI/View/ASpacerFixed.h"

using namespace ass;
using namespace declarative;

class MainWindow : public AWindow {
public:
    MainWindow() : AWindow("Button", 600_dp, 300_dp) {
        setContents(Vertical {
          AText::fromString("Baselines of all views below must be on the same line."),
          Horizontal {
            Label { "Label" },
            AText::fromString("Text"),
            Button { Label { "Button" } },
            Button { Horizontal { Icon { ":img/logo.svg" }, SpacerFixed { 2_dp }, Label { "Button" } } },
            _new<ATextField>() AUI_LET { it->setText("Text field"); },
            _new<ATextArea>() AUI_LET { it->setText("Text area"); },
          },
        });
    }
};

AUI_ENTRY {
    _new<MainWindow>()->show();
    return 0;
}
