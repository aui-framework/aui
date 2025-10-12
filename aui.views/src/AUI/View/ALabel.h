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

#include "AAbstractLabel.h"
#include <AUI/Util/Declarative/Contracts.h>

/**
 * ---
 * title: Label
 * icon: material/format-text-variant
 * ---
 *
 * @brief Represents a simple single-line text display view.
 *
 * ![](imgs/views/ALabel.png)
 *
 * @ingroup views_indication
 * @details
 * Label is a basic UI component designed primarily for displaying text in a single line. While you can add line breaks
 * using `\n` to create multi-line text, it's better to use [AText] instead, which has proper line-breaking
 * capabilities.
 *
 * <!-- aui:snippet examples/7guis/temperature_converter/src/main.cpp window -->
 *
 * Labels can be used to populate AButton (which is a bare container):
 *
 * <!-- aui:snippet examples/ui/button_icon/src/main.cpp AButton_example -->
 *
 * ## API surface
 *
 * <!-- aui:steal_documentation declarative::Label -->
 *
 * ## Coloring a label
 *
 * <!-- aui:snippet examples/ui/views/src/ExampleWindow.cpp label_coloring -->
 *
 * ## Dynamic text in a label
 *
 * You can use [property-system] to bind a label to a variable:
 *
 * <!-- aui:snippet examples/7guis/counter/src/main.cpp counter -->
 *
 * [Full example](7guis-counter.md)
 *
 * ![](imgs/Screenshot_20250401_081123.png)
 *
 */
class API_AUI_VIEWS ALabel: public AAbstractLabel {
public:
    using AAbstractLabel::AAbstractLabel;
};


/// [ALabel_declarative_example]
namespace declarative {
/**
 * <!-- aui:no_dedicated_page -->
 */
struct Label {
    /**
     * @brief Text to display.
     */
    AOptional<contract::In<AString>> text;

    _<ALabel> operator()() {
        auto label = _new<ALabel>();
        AUI_NULLSAFE(text)->bindTo(label->text());
        return label;
    }
};
}
/// [ALabel_declarative_example]

template<>
struct ADataBindingDefault<ALabel, AString> {
public:
    static auto property(const _<ALabel>& view) {
        return view->text();
    }

    static void setup(const _<ALabel>& view) {
    }

    static auto getSetter() { return &ALabel::setText; }
};
