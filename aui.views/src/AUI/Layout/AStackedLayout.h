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

#pragma once

#include "ALinearLayout.h"


/**
 * @brief Places views in a stack (along z axis).
 * @ingroup layout_managers
 * @details
 *
 * |             |                               |
 * |-------------|-------------------------------|
 * | Declarative notation | `Stacked { }`<br/> `Centered { }` |
 *
 * Stacked layout manager places views stacking them onto each other. If the @ref AView "view" is not expanding,
 * the @ref AView "view" is centered.
 *
 * <img src="https://github.com/aui-framework/aui/raw/master/docs/imgs/stacked2.jpg">
 */
class API_AUI_VIEWS AStackedLayout : public ALinearLayout<> {
public:
    AStackedLayout() {
    }

    virtual ~AStackedLayout() = default;

    void onResize(int x, int y, int width, int height) override;

    int getMinimumWidth() override;

    int getMinimumHeight() override;
};
