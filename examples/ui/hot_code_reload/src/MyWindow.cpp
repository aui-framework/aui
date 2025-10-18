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

using namespace ass;
using namespace declarative;

namespace {
class TestRender: public AView {
public:
    TestRender() {
        setCustomStyle({
          FixedSize { 100_dp, 100_dp },
        });
    }
    void render(ARenderContext ctx) override {
        AView::render(ctx);
        // things that render every frame don't even require observing AHotCodeReload::inst().patchEnd.
        ctx.render.setColor(AColor::RED);
        static int frame = 0; // bss test
        ctx.render.string({0, 60}, fmt::format("frame = {}", frame++));
    }
};
}


// [AHotCodeReload_example]
#if __has_include(<AUI/Remote/AHotCodeReload.h>)
#include <AUI/Remote/AHotCodeReload.h>
#endif


MyWindow::MyWindow() : AWindow("Hot code reload", 600_dp, 300_dp) {
    inflate();
#if __has_include(<AUI/Remote/AHotCodeReload.h>)
    AObject::connect(AHotCodeReload::inst().patchEnd, me::inflate);
#endif
}

void MyWindow::inflate() {
    // you can update any things here because inflate is connected to AHotCodeReload::inst().patchEnd.
    setContents(
        Vertical {
          _new<TestRender>(),
          Label { "Hello, world" },
          _new<AButton>("Click me") AUI_LET {
                  AObject::connect(it->clicked, AObject::GENERIC_OBSERVER, [] {
                      AMessageBox::show(nullptr, "Hello", "Hello, world!");
                  });
              },
          CheckBox {
            .checked = AUI_REACT(mChecked),
            .onCheckedChange = [this](bool v) { mChecked = v; },
            .content = Label { "Try check me" },
          },
        } AUI_WITH_STYLE { LayoutSpacing { 4_dp } });
}
// [AHotCodeReload_example]
