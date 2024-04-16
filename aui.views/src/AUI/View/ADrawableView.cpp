// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2024 Alex2772 and Contributors
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

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

void ADrawableView::render(ClipOptimizationContext context) {
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