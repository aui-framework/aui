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

#include <AUI/Platform/SoftwareRenderingContext.h>
#include <AUI/Render/IRendererBackend.h>
#include "AUI/Render/ARender/Software/SoftwareRenderer.h"
#include <AUI/Render/ARender/ADisplayListCanvas.hpp>
#include <AUI/Render/RendererCanvas.h>
#include <AUI/Common/AByteBuffer.h>
#include <AUI/Image/AImage.h>

SoftwareRenderingContext::SoftwareRenderingContext() {

}

SoftwareRenderingContext::~SoftwareRenderingContext() {

}

IRendererBackend& SoftwareRenderingContext::backend() {
    return *mRenderer;
}

void SoftwareRenderingContext::destroyNativeWindow(ASurface &window) {
    CommonRenderingContext::destroyNativeWindow(window);
}

void SoftwareRenderingContext::beginPaint(ASurface &window) {
    CommonRenderingContext::beginPaint(window);
    mDrawList.clear();
    mWindowTarget = mRenderer->createFramebufferWrapper(mBitmapSize, { reinterpret_cast<uint8_t*>(mBitmapBlob.data()), mBitmapSize.x * mBitmapSize.y * 4 });
}

void SoftwareRenderingContext::endPaint(ASurface &window) {
    mDrawList.optimize();
    mDrawList.draw(*mRenderer, mWindowTarget);
    mDrawList.clear();
    CommonRenderingContext::endPaint(window);
}

void SoftwareRenderingContext::beginResize(ASurface &window) {

}

void SoftwareRenderingContext::init(const IRenderingContext::Init &init) {
    CommonRenderingContext::init(init);
    mRenderer = _new<SoftwareRenderer>();
    mCanvas = std::make_unique<ADisplayListCanvas>(mDrawList, *mRenderer);

    mRendererWrapper = std::make_unique<RendererCanvas>(*mCanvas, *mRenderer);
}

void SoftwareRenderingContext::endResize(ASurface &window) {
}

AImage SoftwareRenderingContext::makeScreenshot() {
    return mRenderer->readback(mWindowTarget);
}

void SoftwareRenderingContext::reallocate(const ASurface& window) {
    mBitmapSize = window.getSize();
}

void SoftwareRenderingContext::reallocate() {

}
