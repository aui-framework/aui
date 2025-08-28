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
 * ![](imgs/Views/ATextField.gif)
 *
 * @ingroup views_input
 * @details
 * ![](imgs/Views/ATextField.gif)
 */
class API_AUI_VIEWS ATextField : public AAbstractTextField {
public:
    ATextField();
    ~ATextField() override;

protected:
    bool isValidText(const AString& text) override;
};

template <>
struct ADataBindingDefault<ATextField, AString> {
public:
    static auto property(const _<ATextField>& view) { return view->text(); }

    static void setup(const _<ATextField>& view) {}
    static auto getGetter() { return (ASignal<AString> ATextField::*) &ATextField::textChanging; }
    static auto getSetter() { return (void(ATextField::*)(const AString& t)) & ATextField::setText; }
};
