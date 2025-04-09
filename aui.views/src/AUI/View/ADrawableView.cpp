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

#include <iostream>
#include "ADrawableView.h"
#include <AUI/Render/IRenderer.h>
#include <AUI/ASS/ASS.h>
#include <AUI/Util/AImageDrawable.h>

ADrawableView::ADrawableView(_<IDrawable> drawable) : mDrawable(std::move(drawable)) {

}

void ADrawableView::render(ARenderContext context) {
    AView::render(context);
    AUI_ASSERTX(!getAssHelper()->state.backgroundUrl.image, "BackgroundImage url of ADrawableView should be empty");
    if (!mDrawable) {
        return;
    }

    ass::prop::Property<ass::BackgroundImage>::draw(context, this, mDrawable, getAssHelper()->state.backgroundUrl);
}

ADrawableView::ADrawableView(const AUrl& url): ADrawableView(IDrawable::fromUrl(url)) {

}

ADrawableView::ADrawableView() {

}

/*
void
ADrawableView::userProcessStyleSheet(const std::function<void(css, const std::function<void(property)>&)>& processor) {
    processor(css::T_AUI_BACKGROUND_OVERLAY, [&](property p)
    {
        if (p->getArgs().size() == 1) {
            mColorOverlay = AColor(p->getArgs()[0]);
        }
    });
}
*/