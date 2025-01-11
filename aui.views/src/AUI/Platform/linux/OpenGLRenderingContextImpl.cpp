/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
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
#include <GL/glx.h>


/* Typedef for the GL 3.0 context creation function */
typedef GLXContext (*PFNGLXCREATECONTEXTATTRIBSARBPROC)(Display *dpy,
                                                        GLXFBConfig config,
                                                        GLXContext
                                                        share_context,
                                                        Bool direct,
                                                        const int
                                                        *attrib_list);
static PFNGLXCREATECONTEXTATTRIBSARBPROC glXCreateContextAttribsARB = nullptr;


OpenGLRenderingContext::~OpenGLRenderingContext() {
}

void OpenGLRenderingContext::init(const Init& init) {
    CommonRenderingContext::init(init);

    mRenderer = ourRenderer();
}

void OpenGLRenderingContext::destroyNativeWindow(AWindowBase& window) {
    CommonRenderingContext::destroyNativeWindow(window);
}

void OpenGLRenderingContext::beginPaint(AWindowBase& window) {
    CommonRenderingContext::beginPaint(window);

//    if (auto w = dynamic_cast<AWindow*>(&window)) {
//        glXMakeCurrent(ourDisplay, w->mHandle, ourContext);
//    }
    beginFramebuffer(window.getSize());
    mRenderer->beginPaint(window.getSize());
}

void OpenGLRenderingContext::endPaint(AWindowBase& window) {
    CommonRenderingContext::endPaint(window);
    endFramebuffer();
    mRenderer->endPaint();
//    if (auto w = dynamic_cast<AWindow*>(&window)) {
//        glXSwapBuffers(ourDisplay, w->mHandle);
//    }
}

void OpenGLRenderingContext::beginResize(AWindowBase& window) {
    if (auto w = dynamic_cast<AWindow*>(&window)) {
//        glXMakeCurrent(ourDisplay, w->mHandle, ourContext);
    }
}

void OpenGLRenderingContext::endResize(AWindowBase& window) {

}

AImage OpenGLRenderingContext::makeScreenshot() {
    return AImage();
}
