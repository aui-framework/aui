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
#include "AUI/Common/AByteBufferView.h"
#include "AUI/Image/AImage.h"
#include "AUI/Image/APixelFormat.h"
#include "AUI/Software/SoftwareRenderer.h"
#include <AUI/Render/ADisplayListCanvas.hpp>
#include <AUI/Render/CanvasRenderer.h>

SoftwareRenderingContext::SoftwareRenderingContext() {}

SoftwareRenderingContext::~SoftwareRenderingContext() {
    if (mBitmapBlob) {
        free(mBitmapBlob);
        mBitmapBlob = nullptr;
    }
}

IRendererBackend& SoftwareRenderingContext::backend() {
    return *mRenderer;
}

void SoftwareRenderingContext::init(const Init& init) {
    CommonRenderingContext::init(init);
    mRenderer = _new<SoftwareRenderer>();
    mCanvas = std::make_unique<ADisplayListCanvas>(mDisplayList, *mRenderer);
    mRendererWrapper = std::make_unique<CanvasRenderer>(*mCanvas);
}

void SoftwareRenderingContext::beginPaint(ASurface &window) {
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

void SoftwareRenderingContext::beginResize(ASurface &window) {}


void SoftwareRenderingContext::endResize(ASurface &window) { reallocate(window); }

void SoftwareRenderingContext::reallocate(const ASurface &window) {
    mBitmapSize = window.getSize();
    reallocate();
}

void SoftwareRenderingContext::reallocate() {
    if (mBitmapBlob) {
        free(mBitmapBlob);
    }
    mBitmapBlob = static_cast<uint8_t *>(malloc(mBitmapSize.x * mBitmapSize.y * 4));

    mStencilBlob.reallocate(mBitmapSize.x * mBitmapSize.y);
}

AImage SoftwareRenderingContext::makeScreenshot() {
    AByteBuffer data;
    size_t s = mBitmapSize.x * mBitmapSize.y * 4;
    data.resize(s);
    std::memcpy(data.data(), mBitmapBlob, s);
    return AImageView(data, mBitmapSize, APixelFormat::BGRA | APixelFormat::BYTE).convert(APixelFormat::RGBA_BYTE);
}
