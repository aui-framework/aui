/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

//
// Created by alex2 on 5/22/2021.
//


#pragma once


#include "AViewContainer.h"
#include "AAbstractTextField.h"
#include "AScrollbar.h"

/**
 * @brief Multiline text field.
 * @ingroup useful_views
 * @details
 * Word breaking text area.
 */
class API_AUI_VIEWS ATextArea: public AViewContainer {
private:
    class TextAreaField;
    _<TextAreaField> mTextField;
    _<AScrollbar> mScrollbar;
    bool mEditable = false; // TODO editable

public:
    ATextArea();
    explicit ATextArea(const AString& text);

    int getContentMinimumHeight(ALayoutDirection layout) override;

    void onScroll(const AScrollEvent& event) override;
};


