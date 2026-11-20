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
#include <emscripten/html5_webgl.h>


void OpenGLRenderingContext::init(const Init& init) {
    CommonRenderingContext::init(init);
    EmscriptenWebGLContextAttributes atrs;
    emscripten_webgl_init_context_attributes(&atrs);
    atrs.alpha = true;
    atrs.depth = true;
    // the renderer clips views with the stencil buffer
    atrs.stencil = true;
    atrs.antialias = false;
    atrs.majorVersion = 2;
    // otherwise the canvas is cleared once presented and makeScreenshot() gets a blank picture.
    atrs.preserveDrawingBuffer = true;

    auto emctx = emscripten_webgl_create_context("#canvas", &atrs);
    if (emctx <= 0) {
        // no WebGL 2 (old Safari): the renderer needs GLES 2 only
        atrs.majorVersion = 1;
        emctx = emscripten_webgl_create_context("#canvas", &atrs);
    }
    if (emctx <= 0) {
        throw AException("WebGL is not available");
    }
    emscripten_webgl_make_context_current(emctx);
    // gl.h is glad: its function pointers are empty until loaded
    if (!OpenGLRenderer::loadGL((OpenGLRenderer::GLLoadProc)emscripten_webgl_get_proc_address, true)) {
        throw AException("Failed to load WebGL functions");
    }

    mRenderer = ourRenderer();
}

void OpenGLRenderingContext::destroyNativeWindow(ASurface& window) {
    CommonRenderingContext::destroyNativeWindow(window);
}

void OpenGLRenderingContext::beginPaint(ASurface& window) {
    mViewportSize = window.getSize();
    bindViewport();
    mRenderer->beginPaint(window.getSize());
}

void OpenGLRenderingContext::beginResize(ASurface& window) {
}

void OpenGLRenderingContext::endResize(ASurface& window) {

}

void OpenGLRenderingContext::endPaint(ASurface& window) {
    mRenderer->endPaint();
    CommonRenderingContext::endPaint(window);
}

OpenGLRenderingContext::~OpenGLRenderingContext() {
}
