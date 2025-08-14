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

#include "AFontStyle.h"

/**
 * @brief Interface of a AView that works with fonts (i.e., ALabel, ATextField, AText, etc...)
 */
class API_AUI_VIEWS IFontView {
public:
    virtual ~IFontView();

    [[nodiscard]]
    AFontStyle& getFontStyle() {
        return mFontStyle;
    }

    virtual void invalidateFont() = 0;

protected:
    void invalidateAllStylesFont();
    void commitStyleFont();

private:
    AFontStyle mFontStyle, mPrevFontStyle { .font = nullptr };
};