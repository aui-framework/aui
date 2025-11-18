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
// Created by Alex2772 on 12/7/2021.
//

#include <AUI/GL/gl.h>
#include <AUI/Platform/OpenGLRenderingContext.h>
#include <AUI/Util/ARandom.h>
#include <AUI/Logging/ALogger.h>
#include <AUI/Platform/AMessageBox.h>
#include <AUI/GL/GLDebug.h>

#include <AUI/GL/OpenGLRenderer.h>
#include <AUI/GL/State.h>
#include <EGL/egl.h>


void OpenGLRenderingContext::init(const Init& init) {
    CommonRenderingContext::init(init);
    gladLoadGLES2Loader(reinterpret_cast<GLADloadproc>(eglGetProcAddress));
    mRenderer = ourRenderer();
}

void OpenGLRenderingContext::destroyNativeWindow(AWindowBase& window) {
    CommonRenderingContext::destroyNativeWindow(window);
}

void OpenGLRenderingContext::beginPaint(AWindowBase& window) {
    beginFramebuffer(window.getSize());
    mRenderer->beginPaint(window.getSize());
}

void OpenGLRenderingContext::beginResize(AWindowBase& window) {
}

void OpenGLRenderingContext::endResize(AWindowBase& window) {

}

void OpenGLRenderingContext::endPaint(AWindowBase& window) {
    endFramebuffer();
    mRenderer->endPaint();
    CommonRenderingContext::endPaint(window);
}

OpenGLRenderingContext::~OpenGLRenderingContext() {
}

AImage OpenGLRenderingContext::makeScreenshot() {
    // stub
    return AImage();
}
