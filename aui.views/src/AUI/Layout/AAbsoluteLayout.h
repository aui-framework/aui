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

#include "ALinearLayout.h"

namespace aui::detail {
    struct AbsoluteLayoutCell {
        aui::non_null<_<AView>> view;
        AOptional<AMetric> pivotX, pivotY;
        AOptional<AMetric> sizeX, sizeY;

        operator _<AView>() const {
            return view;
        }
    };
}

/**
 * @brief Absolute positioning layout. Allows to explicitly set your own coordinates.
 * @ingroup layout-managers
 */
class API_AUI_VIEWS AAbsoluteLayout: public ALinearLayout<aui::detail::AbsoluteLayoutCell> {
public:
    void onResize(int x, int y, int width, int height) override;

    void add(aui::detail::AbsoluteLayoutCell cell);

    void addView(const _<AView>& view, AOptional<size_t> index) override;

    int getMinimumWidth() override;
    int getMinimumHeight() override;

private:
    using ViewInfo = aui::detail::AbsoluteLayoutCell;
};

