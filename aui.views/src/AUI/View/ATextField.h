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

#include "AView.h"
#include "AAbstractTextField.h"
#include <AUI/Util/ADataBinding.h>

/**
 * @brief Editable field with text to receive a text input from the user.
 *
 * ![](imgs/views/ATextField.gif)
 *
 * @ingroup views_input
 * @details
 * Single-line text input. For multiple lines see [ATextArea].
 *
 * ## Reading and writing the text
 *
 * `text()` is a property; it can be read, assigned, observed, and bound to other properties with `&&` (two-way) or
 * `connect`. Whenever you change it from code, the field is updated.
 *
 * <!-- aui:snippet examples/7guis/crud/src/main.cpp crud_form -->
 *
 * Two signals report changes:
 *
 * - `textChanging` is emitted on every typed or removed symbol. This is what `text().changed` of the property refers
 *   to, and the signal that data bindings listen to;
 * - `textChanged` is emitted when the user finishes editing (the field loses focus) and when the text is set with
 *   `setText`.
 *
 * ## Validation
 *
 * The field itself accepts any text; to validate, observe the text and style the field accordingly. The example below
 * parses the text as a date and paints the field red if parsing fails.
 *
 * <!-- aui:snippet examples/7guis/flight_booker/src/main.cpp date_field -->
 *
 * ## Other options
 *
 * - `setPasswordMode(true)` hides the typed symbols and disables copying;
 * - `setEditable(false)` makes the field read-only;
 * - `setMaxTextLength(...)` limits the length of the text;
 * - `setTextInputType(...)` and `setTextInputActionIcon(...)` hint the touchscreen keyboard;
 * - `focus()` (from [AView]) puts the cursor to the field, i.e., `_new<ATextField>() AUI_LET { it->focus(); }`.
 *
 * Clipboard operations are available as `copyToClipboard()`, `cutToClipboard()` and `pasteFromClipboard()`; see also
 * [AClipboard].
 */
class API_AUI_VIEWS ATextField : public AAbstractTextField {
public:
    ATextField();
    ~ATextField() override;

protected:
    bool isValidText(std::u32string_view text) override;
};

template <>
struct ADataBindingDefault<ATextField, AString> {
public:
    static auto property(const _<ATextField>& view) { return view->text(); }

    static void setup(const _<ATextField>& view) {}
    static auto getGetter() { return (ASignal<AString> ATextField::*) &ATextField::textChanging; }
    static auto getSetter() { return (void(ATextField::*)(const AString& t)) & ATextField::setText; }
};
