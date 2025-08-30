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

//
// Created by alex2 on 6/25/2021.
//


#pragma once

#include <AUI/View/AView.h>
#include <AUI/Util/ALayoutDirection.h>
#include <AUI/GL/Vao.h>
#include "AUI/Font/IFontView.h"

/**
 * @brief A Photoshop-like ruler for measuring display units.
 *
 * ![](imgs/views/ARulerView.png)
 *
 * @ingroup views_containment
 */
class API_AUI_VIEWS ARulerView: public AView, public IFontView {
private:
    ALayoutDirection mLayoutDirection;
    _<gl::Vao> mPrecompiledLines;

    int mOffsetPx = 0;

    float mUnit = 1.f;

    int getLongestSide() const;
    int getShortestSide() const;

public:
    explicit ARulerView(ALayoutDirection layoutDirection);

    void invalidateFont() override;

    void render(ARenderContext ctx) override;

    void setOffsetPx(int offsetPx) {
        mOffsetPx = offsetPx;
        redraw();
    }

};


