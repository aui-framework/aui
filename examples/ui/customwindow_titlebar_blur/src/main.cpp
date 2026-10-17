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

#include "AUI/Platform/AMessageBox.h"
#include "AUI/View/AButton.h"

#include <AUI/Platform/Entry.h>
#include <AUI/Platform/AWindow.h>
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/AScrollArea.h>
#include <AUI/View/ATitlebar.h>
#include <AUI/View/ADrawableView.h>

using namespace declarative;
using namespace ass;

AUI_ENTRY {
    auto window = _new<AWindow>(AWindow::Init {
        .title = "Custom window titlebar blur",
        .width = 600_dp,
        .height = 500_dp,
        .decoration = AWindow::Init::DecorationExtended {},
    });

    window->setContents(Stacked {
        // the scroll area takes the whole window, including the titlebar area.
        AScrollArea::Builder()
                .withContents(Centered {
                    Icon { ":logo.svg" } AUI_OVERRIDE_STYLE { FixedSize { 1024_dp } },
                })
                .build() AUI_OVERRIDE_STYLE { Expanding() },

        // the titlebar is overlaid on top of the scroll area.
        Vertical::Expanding {
            Titlebar {
                Horizontal {
                    Centered {
                        Button {
                            .content = Label { "Before" },
                            .onClick = [w = window.get()] { AMessageBox::show(w, "Test", "Test"); },
                        },
                    },
                    // shows window->title() and is aligned the way the OS aligns titles.
                    WindowTitle {},
                    Centered {
                        Button {
                            .content = Label { "After" },
                            .onClick = [w = window.get()] { AMessageBox::show(w, "Test", "Test"); },
                        },
                    },
                },
            } AUI_OVERRIDE_STYLE {
                    Backdrop {
                        Backdrop::GaussianBlur { 32_dp },
                    },
                    BackgroundSolid { AColor::WHITE.transparentize(0.5f) },
                },
            SpacerExpanding {},
        },
    });

    window->show();
    return 0;
}
