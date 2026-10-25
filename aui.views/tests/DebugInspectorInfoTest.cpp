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

#include <gmock/gmock.h>
#include "AUI/UITest.h"
#include "AUI/View/ACheckBox.h"
#include "AUI/View/ARadioButton.h"
#include "AUI/View/AProgressBar.h"
#include "AUI/View/ADropdownList.h"
#include "AUI/View/ATextField.h"

namespace {
AString valueOf(IInspectable& view, const AString& name) {
    for (const auto& [key, value] : view.debugInspectorInfo()) {
        if (key == name) {
            return value.renderAsString();
        }
    }
    return "<missing>";
}
}   // namespace

class DebugInspectorInfoTest : public testing::UITest {};

TEST_F(DebugInspectorInfoTest, TextField) {
    auto field = _new<ATextField>();
    field->setText("hello");
    EXPECT_EQ(valueOf(*field, "text"), "hello");
}

TEST_F(DebugInspectorInfoTest, CheckBox) {
    auto checkBox = _new<ACheckBox>();
    EXPECT_EQ(valueOf(*checkBox, "checked"), "false");
    checkBox->checked() = true;
    EXPECT_EQ(valueOf(*checkBox, "checked"), "true");
}

TEST_F(DebugInspectorInfoTest, RadioButton) {
    auto radio = _new<ARadioButton>();
    EXPECT_EQ(valueOf(*radio, "checked"), "false");
    radio->checked() = true;
    EXPECT_EQ(valueOf(*radio, "checked"), "true");
}

TEST_F(DebugInspectorInfoTest, ProgressBar) {
    auto bar = _new<AProgressBar>();
    bar->setValue(0.5f);
    EXPECT_EQ(valueOf(*bar, "value"), "0.5");
}

TEST_F(DebugInspectorInfoTest, KeepsBaseViewInfo) {
    auto checkBox = _new<ACheckBox>();
    EXPECT_NE(valueOf(*checkBox, "Size"), "<missing>");
}
