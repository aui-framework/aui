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
// Created by alex2 on 23.10.2020.
//

#pragma once

#include <AUI/Image/IDrawable.h>
#include <AUI/Util/Declarative.h>
#include "AView.h"

/**
 * @brief Simple view to draw an IDrawable.
 * @ingroup useful_views
 */
class API_AUI_VIEWS ADrawableView: public AView {
public:
    explicit ADrawableView(const AUrl& url);
    explicit ADrawableView(const _<IDrawable>& drawable);
    ADrawableView();
    void render(ARenderContext context) override;

    void setDrawable(const _<IDrawable>& drawable) {
        mDrawable = drawable;
        redraw();
    }

    [[nodiscard]]
    const _<IDrawable>& getDrawable() const noexcept {
        return mDrawable;
    }

private:
    _<IDrawable> mDrawable;
};

template<>
struct ADataBindingDefault<ADrawableView, _<IDrawable>> {
public:
    static void setup(const _<ADrawableView>& view) {
    }

    static auto getSetter() { return &ADrawableView::setDrawable; }
};

namespace declarative {
    using Icon = aui::ui_building::view<ADrawableView>;
}
