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

#include <iostream>
#include "ADrawableView.h"
#include <AUI/Render/ARender.h>
#include <AUI/ASS/ASS.h>
#include <AUI/Util/AImageDrawable.h>

ADrawableView::ADrawableView(const _<IDrawable>& drawable) : mDrawable(drawable) {

}

void ADrawableView::render(ARenderContext context) {
    AView::render(context);
    ARender::setColor(getAssHelper()->state.backgroundUrl.overlayColor.or_default(0xffffff_rgb));
    if (mDrawable) {
        IDrawable::Params p;
        p.size = getSize();
        mDrawable->draw(p);
    }
}

ADrawableView::ADrawableView(const AUrl& url): ADrawableView(IDrawable::fromUrl(url)) {

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