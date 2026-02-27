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
/// [colorpicker_example]
#include <AUI/Platform/Entry.h>
#include <AUI/Platform/AWindow.h>
#include <AUI/View/AColorPicker.h>

using namespace ass;
using namespace declarative;


class MainWindow : public AWindow {
public:
    MainWindow() : AWindow("Color picker", 600_dp, 300_dp) {
        auto color = _new<AProperty<AColor>>(AColor::RED);
        setContents(Vertical {
          GroupBox {
            Label { "View" },
            color_picker::ColorView { AUI_REACT(**color) } AUI_OVERRIDE_STYLE {
                  FixedSize(40_dp, 20_dp),
                  Border { 1_px, AColor::BLACK },
                  Padding { 2_px },
                },
          },
          GroupBox {
            Label { "Sliders RGB" },
            color_picker::SlidersRGB {
              .color = AUI_REACT(**color),
              .onColorChanged = [color](AColor c) { *color = c; },
            },
          },
        });
    }

private:
    void onClick() { ALogger::info("Test") << "Hello world!"; }
};

AUI_ENTRY {
    _new<MainWindow>()->show();
    return 0;
}
/// [colorpicker_example]
