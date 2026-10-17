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

#include <AUI/Render/ARender/GL/gl.h>
#include <AUI/Platform/OpenGLRenderingContext.h>
#include <AUI/Util/ARandom.h>
#include <AUI/Logging/ALogger.h>
#include <AUI/Platform/AMessageBox.h>
#include <AUI/Render/ARender/GL/GLDebug.h>

#include <AUI/Render/ARender/GL/OpenGLBackend.hpp>
#include <AUI/Render/ARender/GL/State.h>
#include <emscripten/html5_webgl.h>


void OpenGLRenderingContext::init(const Init& init) {
    CommonRenderingContext::init(init);
    EmscriptenWebGLContextAttributes atrs;
    emscripten_webgl_init_context_attributes(&atrs);
    atrs.alpha = true;
    atrs.depth = true;
    atrs.stencil = false;

    auto emctx = emscripten_webgl_create_context("#canvas", &atrs);
    emscripten_webgl_make_context_current(emctx);

    mRenderer = ourRenderer();
    mCanvas = std::make_unique<ADisplayListCanvas>(mDrawList, *mRenderer);
    mRendererWrapper = std::make_unique<RendererCanvas>(*mCanvas, *mRenderer);
}

void OpenGLRenderingContext::destroyNativeWindow(ASurface& window) {
    CommonRenderingContext::destroyNativeWindow(window);
}

void OpenGLRenderingContext::beginPaint(ASurface& window) {
    mDrawList.clear();
    mViewportSize = window.getSize();
    bindViewport();
    beginFramebuffer(mViewportSize);
}

void OpenGLRenderingContext::beginResize(ASurface& window) {
}

void OpenGLRenderingContext::endResize(ASurface& window) {

}

void OpenGLRenderingContext::endPaint(ASurface& window) {
    mDrawList.optimize();
    mDrawList.draw(*mRenderer, mWindowTarget);
    presentToBackbuffer();
    mDrawList.clear();

    CommonRenderingContext::endPaint(window);
}

OpenGLRenderingContext::~OpenGLRenderingContext() {
}
