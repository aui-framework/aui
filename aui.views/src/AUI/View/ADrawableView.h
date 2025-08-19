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
// Created by alex2 on 23.10.2020.
//

#pragma once

#include <AUI/Image/IDrawable.h>
#include <AUI/Util/Declarative.h>
#include "AView.h"

/**
 * @brief Simple view to draw an IDrawable.
 * @ingroup useful_views
 * @details
 * ADrawableView brings you more control over the drawable. This is useful for drawables with extended interface
 * (i.e., AAnimatedImage).
 *
 *
 * The sizing and positioning acts like ass::BackgroundImage. Moreover, these rules are taken from ass::BackgroundImage
 * applied to ADrawableView:
 * ```cpp
 * auto drawable = IDrawable::fromUrl(":animation.webp");
 * _new<ADrawableView>(drawable) AUI_WITH_STYLE {
 *   BackgroundImage({}, {}, {}, Sizing::COVER), // <- drawable is sized and positioned as with COVER.
 * };
 * ```
 *
 * When applying ass::BackgroundImage, do not pass url to it, it's redundant for ADrawableView and would cause a debug
 * error.
 */
class API_AUI_VIEWS ADrawableView : public AView {
public:
    /**
     * @brief Create an instance with the URL of a image resource.
     * @param url URL to the image resource.
     */
    explicit ADrawableView(const AUrl& url);

    /**
     * @brief Drawable property.
     */
    auto drawable() const {
        return APropertyDef {
            this,
            &ADrawableView::getDrawable,
            &ADrawableView::setDrawable,
            mDrawableChanged,
        };
    }

    /**
     * @brief Create an instance from the given drawable.
     * @param drawable The IDrawable to be rendered.
     */
    explicit ADrawableView(_<IDrawable> drawable);
    ADrawableView();
    ~ADrawableView() override = default;
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
    emits<_<IDrawable>> mDrawableChanged;
};

template <aui::derived_from<ADrawableView> T>
struct ADataBindingDefault<T, _<IDrawable>> {
public:
    static auto property(const _<ADrawableView>& view) { return view->drawable(); }

    static void setup(const _<ADrawableView>& view) {}

    static auto getSetter() { return &ADrawableView::setDrawable; }
};

class ADrawableIconView : public ADrawableView {
public:
    using ADrawableView::ADrawableView;
    ~ADrawableIconView() override = default;
};

namespace declarative {

/**
 * @declarativeformof{ADrawableView}
 */
struct Icon : aui::ui_building::view<ADrawableIconView> {
    using aui::ui_building::view<ADrawableIconView>::view;
};
}   // namespace declarative
