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
//#include <GL/glx.h>


OpenGLRenderingContext::~OpenGLRenderingContext() {}

void OpenGLRenderingContext::beginPaint(ASurface& window) {
    CommonRenderingContext::beginPaint(window);
    beginFramebuffer(window.getSize());
    mRenderer->beginPaint(window.getSize());
}

void OpenGLRenderingContext::endPaint(ASurface& window) {
    CommonRenderingContext::endPaint(window);
    endFramebuffer();
    mRenderer->endPaint();
}

void OpenGLRenderingContext::beginResize(ASurface& window) {
}

void OpenGLRenderingContext::endResize(ASurface& window) {

}

AImage OpenGLRenderingContext::makeScreenshot() {
    if (auto fb = std::get_if<gl::Framebuffer>(&mFramebuffer)) {
        fb->bindForRead();
        AImage result(mViewportSize, APixelFormat::RGBA_BYTE);
        glReadPixels(0, 0, result.width(), result.height(), GL_RGBA, GL_UNSIGNED_BYTE, static_cast<void*>(result.modifiableBuffer().data()));
        result.mirrorVertically();
        gl::Framebuffer::unbind();
        return result;
    }
    return {};
}
