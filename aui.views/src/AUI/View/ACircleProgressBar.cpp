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
// Created by Alex2772 on 1/8/2022.
//

#include "ACircleProgressBar.h"
#include "AUI/Layout/AStackedLayout.h"
#include "AUI/Render/ABrush.h"
#include "AUI/Render/RenderHints.h"
#include "AUI/Util/AAngleRadians.h"
#include "AUI/Render/IRendererBackend.h"
#include "AUI/Render/ACanvas.hpp"

ACircleProgressBar::ACircleProgressBar() : mInner(_new<Inner>()) {
    setLayout(std::make_unique<AStackedLayout>());
    addView(mInner);
}

ACircleProgressBar::Inner::~Inner() {

}

ACircleProgressBar::~ACircleProgressBar() {

}

void ACircleProgressBar::render(ARenderContext context) {
    AView::render(context); // NOLINT(*-parent-virtual-call)
    
    if (mValue > 0.0001f) {
        if (!mMaskTexture || mValue != mValueForMask || getSize() != mSizeForMask) {
            mSizeForMask = getSize();
            mValueForMask = mValue;
            if (mSizeForMask.x > 0 && mSizeForMask.y > 0) {
                if (!mMaskTexture || mMaskTexture->getSize() != glm::u32vec2(mSizeForMask)) {
                    mMaskTexture = context.backend.createTexture(mSizeForMask, APixelFormat::R8_UNORM);
                }
                auto pass = context.backend.beginOffscreen(mMaskTexture);
                auto& c = pass->context().canvas;
                c.clear(AColor::TRANSPARENT_BLACK);
                c.squareSector(APaint{ASolidBrush{AColor::WHITE}}, {0, 0}, mSizeForMask, 0_deg, AAngleRadians(glm::radians(mValue * 360.f)));
                context.backend.endOffscreen(std::move(pass));
            }
        }

        if (mMaskTexture) {
            context.canvas.pushMask(mMaskTexture, glm::vec4(0, 0, mSizeForMask));
            AViewContainerBase::renderChildren(context);
            context.canvas.popMask();
        } else {
            AViewContainerBase::renderChildren(context);
        }
    }
}

_<AView> declarative::CircleProgressBar::operator()() {
    auto view = _new<ACircleProgressBar>();
    progress.bindTo(view->value().assignment());
    return view;
}
