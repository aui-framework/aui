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
#include <AUI/Util/ARaiiHelper.h>
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
        } else if (mRenderToTexture->texture && mRenderToTexture->texture->getSize() != glm::u32vec2(getSize())) {
            mRenderToTexture->texture = nullptr;
            mRenderToTexture->invalidArea.reset();
            mOffscreenRedrawRequested = false;
        }
    } else {
        mRenderToTexture.reset();
        mOffscreenRedrawRequested = false;
    }
}

void AScrollAreaViewport::render(ARenderContext ctx) {
    if (mRenderToTexture) {
        if (!mRenderToTexture->texture || mRenderToTexture->texture->getSize() != glm::u32vec2(getSize())) {
            drawOffscreen(ctx);
        }
        if (mRenderToTexture->texture) {
            ctx.canvas.rectangle(APaint{ATexturedBrush{mRenderToTexture->texture, {}, {}, {}, {}, true}}, {0, 0}, getSize());
        }
        mRedrawRequested = false;
    } else {
        AViewContainerBase::render(ctx);
    }
}

void AScrollAreaViewport::markPixelDataInvalid(ARect<int> invalidArea) {
    if (mRenderToTexture) {
        if (mRenderToTexture->invalidArea) {
            mRenderToTexture->invalidArea->p1 = glm::min(mRenderToTexture->invalidArea->p1, invalidArea.p1);
            mRenderToTexture->invalidArea->p2 = glm::max(mRenderToTexture->invalidArea->p2, invalidArea.p2);
        } else {
            mRenderToTexture->invalidArea = invalidArea;
        }
        if (!std::exchange(mOffscreenRedrawRequested, true)) {
            AWindow::current()->beforeFrameQueue().enqueue([this, self = aui::ptr::shared_from_this(this)](ARenderContext rc) {
                if (!mRenderToTexture) {
                    mOffscreenRedrawRequested = false;
                    return;
                }
                if (!mRenderToTexture->invalidArea) {
                    mOffscreenRedrawRequested = false;
                    return;
                }
                drawOffscreen(rc);
            });
        }
        AUI_NULLSAFE(mParent)->markPixelDataInvalid(ARect<int>::fromTopLeftPositionAndSize(getPosition(), getSize()));
        return;
    }

    AViewContainerBase::markPixelDataInvalid(invalidArea);
}

void AScrollAreaViewport::drawOffscreen(ARenderContext rc) {
    if (!mRenderToTexture) {
        mOffscreenRedrawRequested = false;
        return;
    }

    if (glm::any(glm::equal(getSize(), glm::ivec2(0)))) {
        mRedrawRequested = false;
        mOffscreenRedrawRequested = false;
        mRenderToTexture->invalidArea.reset();
        return;
    }

    if (!mRenderToTexture->texture || mRenderToTexture->texture->getSize() != glm::u32vec2(getSize())) {
        mRenderToTexture->texture = rc.backend.createTexture(getSize());
        mRenderToTexture->invalidArea.reset();
    }

    auto offscreenPass = rc.backend.beginOffscreen(mRenderToTexture->texture);
    auto contextOfTheView = offscreenPass->context();

    AUI_DEFER {
        rc.backend.endOffscreen(std::move(offscreenPass));
        mRenderToTexture->invalidArea.reset();
        mOffscreenRedrawRequested = false;
    };

    if (mRenderToTexture->invalidArea) {
        ARect<int> expandedArea = mRenderToTexture->invalidArea.value();
        expandedArea.p1 -= 8;
        expandedArea.p2 += 8;
        contextOfTheView.canvas.pushClipRect(ARect<float>::fromTopLeftPositionAndSize(expandedArea.min(), expandedArea.size()));
        APaint clear_paint;
        clear_paint.blending = Blending::CLEAR;
        contextOfTheView.canvas.rectangle(clear_paint, expandedArea.min(), expandedArea.size());
    } else {
        contextOfTheView.canvas.clear();
    }

    try {
        AViewContainerBase::render(contextOfTheView);
    } catch (const AException& e) {
        ALogger::err("AScrollAreaViewport") << "Unable to render viewport: " << e;
    }
}

void AScrollAreaViewport::updateContentsScroll() {
    mInner->setPosition(-glm::ivec2(mScroll));
    emit mScrollChanged(mScroll);
    redraw();
}
