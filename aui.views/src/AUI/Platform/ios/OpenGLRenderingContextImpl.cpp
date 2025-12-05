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

#include <dlfcn.h>
#include <AUI/GL/gl.h>
#include <AUI/Platform/OpenGLRenderingContext.h>
#include <AUI/Util/ARandom.h>
#include <AUI/Logging/ALogger.h>
#include <AUI/Platform/AMessageBox.h>
#include <AUI/GL/GLDebug.h>

#include <AUI/GL/OpenGLRenderer.h>
#include <AUI/GL/State.h>

static void* UIKit_GL_GetProcAddress(const char *proc)
{
    /* Look through all SO's for the proc symbol.  Here's why:
     * -Looking for the path to the OpenGL Library seems not to work in the iOS Simulator.
     * -We don't know that the path won't change in the future. */
    return dlsym(RTLD_DEFAULT, proc);
}


void OpenGLRenderingContext::init(const Init& init) {
    CommonRenderingContext::init(init);
    gladLoadGLES2Loader(reinterpret_cast<GLADloadproc>(UIKit_GL_GetProcAddress));
    mRenderer = ourRenderer();
}

void OpenGLRenderingContext::destroyNativeWindow(ASurface& window) {
    CommonRenderingContext::destroyNativeWindow(window);
}

void OpenGLRenderingContext::beginPaint(ASurface& window) {
    beginFramebuffer(window.getSize());
    mRenderer->beginPaint(window.getSize());
}

void OpenGLRenderingContext::beginResize(ASurface& window) {
}

void OpenGLRenderingContext::endResize(ASurface& window) {

}

void OpenGLRenderingContext::endPaint(ASurface& window) {
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
