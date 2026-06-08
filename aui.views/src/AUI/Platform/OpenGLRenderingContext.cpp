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

#include "OpenGLRenderingContext.h"
#include <AUI/GL/gl.h>

static constexpr auto LOG_TAG = "OpenGLRenderingContext";

OpenGLRenderingContext::OpenGLRenderingContext(const ARenderingContextOptions::OpenGL& config) : mConfig(config) {}

void OpenGLRenderingContext::bindContext() {

}

void OpenGLRenderingContext::beginFramebuffer(glm::uvec2 windowSize) {
    mViewportSize = windowSize;
    if (!mBackbufferTarget || mBackbufferTarget->getSize() != mViewportSize) {
        mBackbufferTarget = mRenderer->createFramebufferWrapper(gl::Framebuffer::DEFAULT_FB, mViewportSize);
    }
    if (!mWindowTarget || mWindowTarget->getSize() != mViewportSize) {
        mWindowTarget = mRenderer->createTexture(mViewportSize, APixelFormat::R8G8B8A8_UNORM, TextureFilter::LINEAR);
        mWindowTarget->setOrigin(mFlipY ? TextureOrigin::BOTTOM_LEFT : TextureOrigin::TOP_LEFT);
    }
}

uint32_t OpenGLRenderingContext::getDefaultFb() const noexcept {
    return gl::Framebuffer::DEFAULT_FB;
}

AImage OpenGLRenderingContext::makeScreenshot() {
    bindContext();
    if (mWindowTarget) {
        return mRenderer->readback(mWindowTarget);
    }
    return {};
}

void OpenGLRenderingContext::presentToBackbuffer() {
    if (!mWindowTarget || !mBackbufferTarget) {
        return;
    }

    mPresentDisplayList.clear();
    ADisplayListCanvas canvas(mPresentDisplayList, *mRenderer);
    canvas.clear(AColor::TRANSPARENT_BLACK);
    canvas.rectangle(
        APaint {
            .brush = ATexturedBrush {
                .texture = mWindowTarget,
                .imageRendering = ImageRendering::SMOOTH,
                .premultiplied = true,
            },
            //.blending = Blending::CLEAR,
        },
        {0.f, 0.f},
        glm::vec2(mViewportSize));
    mPresentDisplayList.optimize();
    mPresentDisplayList.draw(*mRenderer, mBackbufferTarget);
    mPresentDisplayList.clear();
}
