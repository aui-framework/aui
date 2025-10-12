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

//
// Created by alex2772 on 10/11/25.
//

#include "MyWindow.h"
#include "AUI/Platform/AMessageBox.h"
#include "AUI/View/ACheckBox.h"
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/AButton.h>
#include <AUI/View/AView.h>
#include <AUI/Remote/AHotCodeReload.h>

using namespace ass;
using namespace declarative;

MyWindow::MyWindow() : AWindow("Hot code reload", 600_dp, 300_dp) {
    inflate();
    AObject::connect(AHotCodeReload::inst().patchEnd, me::inflate);
}

void MyWindow::inflate() {
    setContents(
        Vertical {
          Label { "Hello, world" },
          _new<AButton>("Click me") AUI_LET {
                  AObject::connect(it->clicked, AObject::GENERIC_OBSERVER, [] {
                      AMessageBox::show(nullptr, "Hello", "Hello, world!");
                  });
              },
          Button {
            Label { "Reload" },
            [this] {
                AHotCodeReload::inst().loadBinary(
                    "/home/alex2772/CLionProjects/aui/cmake-build-debug/examples/ui/hot_code_reload/CMakeFiles/"
                    "aui.example.hot_code_reload.dir/./src/MyWindow.cpp.o");
            },
          },
          CheckBox {
            .checked = AUI_REACT(mChecked),
            .onCheckedChange = [this](bool v) { mChecked = v; },
            .content = Label { "Try check me" },
          },
          //                        Label { "Hello, world!" },
        } AUI_WITH_STYLE { LayoutSpacing { 4_dp } });
}
