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
#include "AUI/Software/SoftwareRenderer.h"

SoftwareRenderingContext::SoftwareRenderingContext() {

}

SoftwareRenderingContext::~SoftwareRenderingContext() {

}

void SoftwareRenderingContext::destroyNativeWindow(ASurface &window) {
    CommonRenderingContext::destroyNativeWindow(window);
}

void SoftwareRenderingContext::beginPaint(ASurface &window) {
    CommonRenderingContext::beginPaint(window);
    std::memset(mStencilBlob.data(), 0, mStencilBlob.getSize());
}

void SoftwareRenderingContext::endPaint(ASurface &window) {
    CommonRenderingContext::endPaint(window);
}

void SoftwareRenderingContext::beginResize(ASurface &window) {

}

void SoftwareRenderingContext::init(const IRenderingContext::Init &init) {
    CommonRenderingContext::init(init);
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

IRenderer& SoftwareRenderingContext::renderer() {
    static SoftwareRenderer r;
    return r;
}
