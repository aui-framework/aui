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
#include <AUI/Software/SoftwareRenderer.h>

SoftwareRenderingContext::SoftwareRenderingContext() {

}

SoftwareRenderingContext::~SoftwareRenderingContext() {

}

void SoftwareRenderingContext::destroyNativeWindow(AWindowBase &window) {
    CommonRenderingContext::destroyNativeWindow(window);
}

void SoftwareRenderingContext::beginPaint(AWindowBase &window) {
    CommonRenderingContext::beginPaint(window);
}

void SoftwareRenderingContext::endPaint(AWindowBase &window) {
    CommonRenderingContext::endPaint(window);
}

void SoftwareRenderingContext::beginResize(AWindowBase &window) {

}

void SoftwareRenderingContext::init(const IRenderingContext::Init &init) {
    CommonRenderingContext::init(init);
}

void SoftwareRenderingContext::endResize(AWindowBase &window) {

}
AImage SoftwareRenderingContext::makeScreenshot() {
    return AImage{};
}
void SoftwareRenderingContext::reallocateImageBuffers(const AWindowBase& window) {
    mBitmapSize = window.getSize();
}

IRenderer& SoftwareRenderingContext::renderer() {
    static SoftwareRenderer r;
    return r;
}
