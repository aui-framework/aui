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
#include <AUI/View/AView.h>
#include <AUI/Remote/AHotCodeReload.h>

using namespace ass;
using namespace declarative;


class MyWindow: public AWindow {
public:
    MyWindow(): AWindow("Hot code reload", 600_dp, 300_dp) {
        inflate();
        AObject::connect(AHotCodeReload::instance().patchEnd, me::inflate);
    }

    void inflate() {
        setContents(Vertical {
          Label { "Hello, world!" } AUI_WITH_STYLE { Margin { {}, 10_dp } },
          Label { "Hello, world!" },






          _new<AButton>("Reload")
              AUI_LET { AObject::connect(it->clicked, AUI_SLOT(AHotCodeReload::instance())::reload); } ,
        });
    }
};

AUI_ENTRY {
    _new<MyWindow>()->show();
    return 0;
}
