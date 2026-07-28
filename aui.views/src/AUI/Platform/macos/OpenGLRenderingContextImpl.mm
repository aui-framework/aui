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
#import <Cocoa/Cocoa.h>
#include <AUI/Util/ARandom.h>
#include <AUI/Logging/ALogger.h>
#include <AUI/GL/GLDebug.h>

#include <AUI/GL/OpenGLRenderer.h>
#include <AUI/GL/State.h>
#include <AUI/Platform/AWindow.h>


void* OpenGLRenderingContext::ourContext = nullptr;


void OpenGLRenderingContext::init(const Init& init) {
    CommonRenderingContext::init(init);
    auto& window = init.window;

    // INITIALIZE OPENGL
    if (ourContext == nullptr) {
        // set up pixel format
        constexpr int kMaxAttributes = 19;
        NSOpenGLPixelFormatAttribute attributes[kMaxAttributes];
        int numAttributes = 0;
        attributes[numAttributes++] = NSOpenGLPFAAccelerated;
        attributes[numAttributes++] = NSOpenGLPFAClosestPolicy;
        attributes[numAttributes++] = NSOpenGLPFADoubleBuffer;
        attributes[numAttributes++] = NSOpenGLPFAOpenGLProfile;
        attributes[numAttributes++] = NSOpenGLProfileVersion3_2Core;
        attributes[numAttributes++] = NSOpenGLPFAColorSize;
        attributes[numAttributes++] = 24;
        attributes[numAttributes++] = NSOpenGLPFAAlphaSize;
        attributes[numAttributes++] = 8;
        attributes[numAttributes++] = NSOpenGLPFADepthSize;
        attributes[numAttributes++] = 24;
        attributes[numAttributes++] = NSOpenGLPFAStencilSize;
        attributes[numAttributes++] = 8;
        attributes[numAttributes++] = NSOpenGLPFASampleBuffers;
        attributes[numAttributes++] = 0;
        // }
        attributes[numAttributes++] = 0;

        auto pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:attributes];
        if (nil == pixelFormat) {
            throw AException("Failed to initialize NSOpenGLPixelFormat");
        }

        // create context
        auto context = [[NSOpenGLContext alloc] initWithFormat:pixelFormat shareContext:nil];
        if (nil == context) {
            [pixelFormat release];
            throw AException("Failed to initialize NSOpenGLContext");
        }
        ourContext = context;
    }

    auto contentView  = [static_cast<NSWindow*>(window.mHandle) contentView];
    [contentView setWantsBestResolutionOpenGLSurface:YES];
    [static_cast<NSOpenGLContext*>(ourContext) setView:contentView];

    [static_cast<NSOpenGLContext*>(ourContext) makeCurrentContext];

    if (!gladLoadGL()) {
        throw AException("glad load failed");
    }

    GLint stencilBits = 0;
    glGetIntegerv(GL_STENCIL_BITS, &stencilBits);

    mRenderer = ourRenderer();
    ALogger::info("OpenGL context is ready");
}

void OpenGLRenderingContext::destroyNativeWindow(ASurface& window) {
    CommonRenderingContext::destroyNativeWindow(window);
}

void OpenGLRenderingContext::beginPaint(ASurface& window) {
    CommonRenderingContext::beginPaint(window);
    if (auto nativeWindow = dynamic_cast<AWindow*>(&window)) {
        auto contentView  = [static_cast<NSWindow*>(nativeWindow->mHandle) contentView];
        [static_cast<NSOpenGLContext*>(ourContext) setView:contentView];
    }

    beginFramebuffer(window.getSize());
    mRenderer->beginPaint(window.getSize());
}

void OpenGLRenderingContext::beginResize(ASurface& window) {
    [static_cast<NSOpenGLContext*>(ourContext) update];
    [static_cast<NSOpenGLContext*>(ourContext) makeCurrentContext];
    if (auto nativeWindow = dynamic_cast<AWindow*>(&window)) {
        auto contentView  = [static_cast<NSWindow*>(nativeWindow->mHandle) contentView];
        [static_cast<NSOpenGLContext*>(ourContext) setView:contentView];
    }
    GLint swapInterval = 0;
    [static_cast<NSOpenGLContext*>(ourContext) setValues:&swapInterval forParameter:NSOpenGLContextParameterSwapInterval];
}

void OpenGLRenderingContext::endResize(ASurface& window) {
    [static_cast<NSOpenGLContext*>(ourContext) update];
    [static_cast<NSOpenGLContext*>(ourContext) makeCurrentContext];
}

void OpenGLRenderingContext::endPaint(ASurface& window) {
    endFramebuffer();
    mRenderer->endPaint();
    [static_cast<NSOpenGLContext*>(ourContext) flushBuffer];
    CommonRenderingContext::endPaint(window);
}

OpenGLRenderingContext::~OpenGLRenderingContext() {
}

AImage OpenGLRenderingContext::makeScreenshot() {
    return AImage();
}
