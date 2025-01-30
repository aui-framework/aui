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

#include <AUI/Platform/AWindow.h>
#include <AUI/Platform/Entry.h>
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/AScrollArea.h>
#include <AUI/View/ASplitter.h>
#include <AUI/View/ATextArea.h>

using namespace declarative;
using namespace ass;

class MainWindow : public AWindow {
public:
    MainWindow() : AWindow("Notes") {
        setContents(Vertical {
          ASplitter::Horizontal().withItems(
              { Vertical {} with_style {
                  MinSize { 100_dp },
                },
                AScrollArea::Builder().withContents(Vertical {
                    _new<ATextArea>("Untitled") with_style {
                        FontSize{18_pt},
                    },
                    _new<ATextArea>("Text"),
                }).build() }),
        });
    }
};

AUI_ENTRY {
    auto w = _new<MainWindow>();
    w->show();
    return 0;
}
