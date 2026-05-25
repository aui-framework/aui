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
#include "AUI/Software/SoftwareRenderer.h"
#include <AUI/Render/ADisplayListCanvas.hpp>
#include <AUI/Render/RendererCanvas.h>

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
    mRenderer->setWindow(&window);
    mDisplayList.clear();
    std::memset(mStencilBlob.data(), 0, mStencilBlob.getSize());
}

void SoftwareRenderingContext::endPaint(ASurface &window) {
    mDisplayList.optimize();
    mDisplayList.draw(*mRenderer);
    mDisplayList.clear();
    CommonRenderingContext::endPaint(window);
}

void SoftwareRenderingContext::beginResize(ASurface &window) {

}

void SoftwareRenderingContext::init(const IRenderingContext::Init &init) {
    CommonRenderingContext::init(init);
    mRenderer = _new<SoftwareRenderer>();
    mCanvas = std::make_unique<ADisplayListCanvas>(mDisplayList, *mRenderer);
    mRendererWrapper = std::make_unique<RendererCanvas>(*mCanvas);
}

void SoftwareRenderingContext::endResize(ASurface &window) {
    mBitmapSize = window.getSize();
}

AImage SoftwareRenderingContext::makeScreenshot() {
    return {};
}

void SoftwareRenderingContext::reallocate(const ASurface& window) {
    mBitmapSize = window.getSize();
}

void SoftwareRenderingContext::reallocate() {

}
