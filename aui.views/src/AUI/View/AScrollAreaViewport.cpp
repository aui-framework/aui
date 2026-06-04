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

#include <AUI/Util/ALayoutInflater.h>
#include "AScrollAreaViewport.h"
#include "AUI/Common/SharedPtrTypes.h"
#include "AUI/Platform/AWindow.h"
#include "AUI/Render/RenderHints.h"
#include "AUI/View/AView.h"

static constexpr auto RENDER_TO_TEXTURE_THRESHOLD_AREA = 128 * 128;

class AScrollAreaViewport::Inner: public AViewContainer {
public:
    Inner() {
        addAssName("AScrollAreaViewportInner");
    }
};

AScrollAreaViewport::AScrollAreaViewport() {
    addAssName("AScrollAreaViewport");
    addView(mInner = _new<Inner>());
}

AScrollAreaViewport::~AScrollAreaViewport() {

}

void AScrollAreaViewport::setContents(_<AView> content) {
    mInner->setPosition({0, 0});
    ALayoutInflater::inflate(mInner, content);
    mContents = std::move(content);
    updateContentsScroll();
}

void AScrollAreaViewport::applyGeometryToChildren() {
    AViewContainerBase::applyGeometryToChildren();
    mInner->setSize(glm::max(mInner->getMinimumSize(), getSize()));

    if (mInner->getSize().x * mInner->getSize().y >= RENDER_TO_TEXTURE_THRESHOLD_AREA) {
        if (!mRenderToTexture) {
            mRenderToTexture.emplace();
        }
    } else {
        mRenderToTexture.reset();
    }
}

void AScrollAreaViewport::render(ARenderContext ctx) {
    if (mRenderToTexture && mRenderToTexture->drawFromTexture) {
        mRenderToTexture->skipRedrawUntilTextureIsPresented = false;
        if (!mRenderToTexture->texture) {
            drawOffscreen(ctx);
        }
        ctx.canvas.rectangle(APaint{ATexturedBrush{mRenderToTexture->texture, {}, {}, {}, {}, true}}, {0, 0}, getSize());
    } else {
        //AViewContainerBase::render(ctx);
    }
}

void AScrollAreaViewport::markPixelDataInvalid(ARect<int> invalidArea) {
    if (mRenderToTexture) {
        mRenderToTexture->invalidArea << invalidArea;
        if (std::exchange(mRedrawRequested, true)) {
            // this view already requested a redraw.
            return;
        }
        // temporary disable drawing from texture. this will be set back to true by the callback below.
        mRenderToTexture->drawFromTexture = false;
        AWindow::current()->beforeFrameQueue().enqueue([this, self = aui::ptr::shared_from_this(this)](ARenderContext rc) {
            if (!mRenderToTexture) {
                return;
            }
            if (mRenderToTexture->invalidArea.empty()) {
                return;
            }
            drawOffscreen(rc);
        });
        AUI_NULLSAFE(mParent)->markPixelDataInvalid(ARect<int>::fromTopLeftPositionAndSize(getPosition(), getSize()));
        return;
    }

    AViewContainerBase::markPixelDataInvalid(invalidArea);
}

void AScrollAreaViewport::drawOffscreen(ARenderContext rc) {
    if (!mRenderToTexture) {
        return;
    }

    if (mRenderToTexture->skipRedrawUntilTextureIsPresented) {
        mRedrawRequested = false;
        return;
    }

    if (glm::any(glm::equal(getSize(), glm::ivec2(0)))) {
        mRedrawRequested = false;
        mRenderToTexture->invalidArea.clear();
        return;
    }

    auto invalidArea = std::exchange(mRenderToTexture->invalidArea, {});

    if (!mRenderToTexture->texture || mRenderToTexture->texture->getSize() != glm::u32vec2(getSize())) {
        mRenderToTexture->texture = rc.backend.createTexture(getSize());
    }

    auto offscreenPass = rc.backend.beginOffscreen(mRenderToTexture->texture);
    auto contextOfTheView = offscreenPass->context();
    contextOfTheView.canvas.clear();

    try {
        AViewContainerBase::render(contextOfTheView);
    } catch (const AException& e) {
        ALogger::err("AScrollAreaViewport") << "Unable to render viewport: " << e;
        rc.backend.endOffscreen(std::move(offscreenPass));
        return;
    }

    rc.backend.endOffscreen(std::move(offscreenPass));

    mRenderToTexture->skipRedrawUntilTextureIsPresented = true;
    mRenderToTexture->drawFromTexture = true;
}

void AScrollAreaViewport::updateContentsScroll() {
    mInner->setPosition(-glm::ivec2(mScroll));
    emit mScrollChanged(mScroll);
    redraw();
}
