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
    setContents(Vertical {
      Label { "Hello, world" } AUI_WITH_STYLE {
            Margin { {}, 8_dp },
            BackgroundSolid { AColor::RED },
          },
      _new<AButton>("Click me") AUI_LET {
          AObject::connect(it->clicked, AObject::GENERIC_OBSERVER, [] {
              AMessageBox::show(nullptr, "Hello", "Hello, world!");
          });
      },
      //                  Label { "Hello, world!" },
    });
}
