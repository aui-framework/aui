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


#include <AUI/Util/AViewEntry.h>
#include "ALinearLayout.h"

/**
 * @brief Imitates behaviour of word wrapping, but uses @ref AView "views" instead words
 * @ingroup layout_managers
 */
class API_AUI_VIEWS AWordWrappingLayout: public ALinearLayout<> {
private:
    AVector<AViewEntry> mViewEntry;

public:
    void onResize(int x, int y, int width, int height) override;

    int getMinimumWidth() override;

    int getMinimumHeight() override;

    void addView(const _<AView>& view, AOptional<size_t> index) override;

    void removeView(aui::no_escape<AView> view, size_t index) override;

};


