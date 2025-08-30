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

#pragma once

#include "AUI/View/AView.h"
#include "AUI/Util/UIBuildingHelpers.h"
#include "AUI/Util/kAUI.h"
#include "AUI/View/AGroupBox.h"
#include "AUI/View/AButton.h"
#include "AUI/View/ACheckBox.h"
#include "AUI/View/ARadioGroup.h"
#include "AUI/View/ADropdownList.h"
#include "AUI/View/ADragArea.h"
#include "AUI/View/AProgressBar.h"
#include "AUI/View/ACircleProgressBar.h"
#include "AUI/View/ASlider.h"
#include "AUI/View/ANumberPicker.h"
#include "AUI/View/ATextField.h"
#include "AUI/View/ADoubleNumberPicker.h"
#include "AUI/View/AScrollArea.h"
#include "AUI/View/ATextArea.h"

static _<AView> uiBenchmarkScene() {
    using namespace declarative;
    using namespace ass;

    return Horizontal {
        Vertical {
          // buttons
          GroupBox {
            Label { "Buttons" },
            Vertical {
              _new<AButton>("Common button"),
              _new<AButton>("Default button") AUI_LET { it->setDefault(); },
              _new<AButton>("Disabled button") AUI_LET { it->setDisabled(); },
              Button {
                Label { "Button with icon" },
              },
            },
          },

          // checkboxes
          GroupBox {
            CheckBoxWrapper { Label { "Checkboxes" } } AUI_LET { it->checked() = true; },
            Vertical {
              CheckBoxWrapper { Label { "Unchecked checkbox" } },
              CheckBoxWrapper { Label { "Checked checkbox" } } AUI_LET { it->checked() = true; },
              CheckBoxWrapper { Label { "Disabled checkbox" } } AUI_LET { it->setDisabled(); },
            },
          },

          // radiobuttons
          GroupBox {
            Label { "Radiobuttons" },
            RadioGroup {
              RadioButton { "Radiobutton 1" } AUI_LET { it->checked() = true; },
              RadioButton { "Radiobutton 2" },
              RadioButton { "Radiobutton 3" },
              RadioButton { "Disabled radiobutton" } AUI_LET { it->disable(); },
            },
          },

          // comboboxes
          GroupBox {
            Label { "Comboboxes" },
            Vertical {
              _new<ADropdownList>(AListModel<AString>::make({
                "Combobox 1",
                "Combobox 2",
                "Combobox 3",
                "Combobox 4",
                "Combobox 5",
                "Combobox 6",
              })),
              _new<ADropdownList>(AListModel<AString>::make({ "Disabled combobox" })) AUI_LET { it->setDisabled(); },
            },
          },
          GroupBox {
            Label { "Drag area" },

            _new<ADragArea>() AUI_LET {
                    it AUI_WITH_STYLE {
                        MinSize { 100_dp },
                        Border { 1_px, 0x0_rgb },
                    };
                    it->addView(ADragArea::convertToDraggable(_new<AButton>("Drag me!"), false));
                },
          } AUI_WITH_STYLE { Expanding {} },
        },
        Vertical {
          GroupBox {
            Label { "Window factory" },
            Vertical {
              // CheckBoxWrapper { Label { "Resizeable" } }, TODO
              _new<AButton>("Show window"),
              _new<AButton>("Show window without caption"),
              _new<AButton>("Show window custom caption"),
              _new<AButton>("Close all windows"),
            },
          },

          GroupBox {
            Label { "System dialog" },
            Vertical {
              _new<AButton>("Show file chooser"),
              _new<AButton>("Show folder chooser"),
              _new<AButton>("Message box"),
              _new<AButton>("Cause assertion fail"),
              _new<AButton>("Cause hang"),
              _new<AButton>("Cause access violation"),
            },
          },
        },
        Vertical::Expanding {
          // fields
          GroupBox {
            Label { "Progressbar" },
            Vertical {
              _new<AProgressBar>(),
              _new<ACircleProgressBar>(),
              GroupBox {
                Label { "Slider" },
                Vertical {
                  _new<ASlider>(),
                },
              },
            },
          },
          GroupBox {
            Label { "Scaling factor" },
            Horizontal {
              _new<ANumberPicker>(),
              Label { "x0.25" },
            },
          },
          GroupBox {
            Label { "Fields" },
            Vertical::Expanding {
              Label { "Text field" },
              _new<ATextField>() AUI_LET { it->focus(); },
              Label { "Number picker" },
              _new<ANumberPicker>(),
              _new<ADoubleNumberPicker>(),
              Label { "Text area" },
              AScrollArea::Builder()
                      .withContents(_new<ATextArea>(
                          "AUI Framework - Declarative UI toolkit for modern C++20\n"
                          "Copyright (C) 2020-2025 Alex2772 and Contributors\n"
                          "\n"
                          "SPDX-License-Identifier: MPL-2.0\n"
                          "\n"
                          "This Source Code Form is subject to the terms of the Mozilla "
                          "Public License, v. 2.0. If a copy of the MPL was not distributed with this "
                          "file, You can obtain one at http://mozilla.org/MPL/2.0/."))
                      .build()
                  << ".input-field" AUI_LET { it->setExpanding(); },
            } },
        },
    };
}