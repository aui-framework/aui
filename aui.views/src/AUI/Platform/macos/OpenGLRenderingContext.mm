// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2023 Alex2772
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

//
// Created by Alex2772 on 12/7/2021.
//

#include <AUI/GL/gl.h>
#import <Cocoa/Cocoa.h>
#include <AUI/Platform/OpenGLRenderingContext.h>
#include <AUI/Util/ARandom.h>
#include <AUI/Logging/ALogger.h>
#include <AUI/GL/GLDebug.h>

#include <AUI/GL/OpenGLRenderer.h>
#include <AUI/GL/State.h>
#include <AUI/Platform/AWindow.h>



void OpenGLRenderingContext::init(const Init& init) {
    CommonRenderingContext::init(init);
    auto& window = init.window;

    // INITIALIZE OPENGL

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
    // if (fMSAASampleCount > 1) {
    //     attributes[numAttributes++] = NSOpenGLPFAMultisample;
    //     attributes[numAttributes++] = NSOpenGLPFASampleBuffers;
    //     attributes[numAttributes++] = 1;
    //     attributes[numAttributes++] = NSOpenGLPFASamples;
    //     attributes[numAttributes++] = fMSAASampleCount;
    // } else {
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

    auto contentView  = [static_cast<NSWindow*>(window.mHandle) contentView];
    [contentView setWantsBestResolutionOpenGLSurface:YES];
    [context setView:contentView];
    //assert(glGetError() == 0);

    [context makeCurrentContext];

    GLint stencilBits = 0;
    glGetIntegerv(GL_STENCIL_BITS, &stencilBits);

    if (!glewExperimental) {
        glewExperimental = true;
        if (glewInit() != GLEW_OK) {
            throw AException("glewInit failed");
        }
    }
    ARender::setRenderer(mRenderer = ourRenderer());
    ALogger::info("OpenGL context is ready");
    mContext = context;
    //assert(("no stencil bits" && stencilBits > 0));
}

void OpenGLRenderingContext::destroyNativeWindow(ABaseWindow& window) {
    CommonRenderingContext::destroyNativeWindow(window);
    //wglMakeCurrent(nullptr, nullptr);
}

void OpenGLRenderingContext::beginPaint(ABaseWindow& window) {
    CommonRenderingContext::beginPaint(window);
    //bool ok = wglMakeCurrent(mPainterDC, ourHrc);
    //assert(ok);

    mRenderer->beginPaint(window.getSize());
}

void OpenGLRenderingContext::beginResize(ABaseWindow& window) {
    //wglMakeCurrent(mWindowDC, ourHrc);
    GLint swapInterval = 0;
    [static_cast<NSOpenGLContext*>(mContext) setValues:&swapInterval forParameter:NSOpenGLCPSwapInterval];
}

void OpenGLRenderingContext::endResize(ABaseWindow& window) {
    [static_cast<NSOpenGLContext*>(mContext) update];
    [static_cast<NSOpenGLContext*>(mContext) makeCurrentContext];
}

void OpenGLRenderingContext::endPaint(ABaseWindow& window) {
    mRenderer->endPaint();
    //SwapBuffers(mPainterDC);
    //wglMakeCurrent(mWindowDC, ourHrc);
    [static_cast<NSOpenGLContext*>(mContext) flushBuffer];
    CommonRenderingContext::endPaint(window);
}

OpenGLRenderingContext::~OpenGLRenderingContext() {
}

AImage OpenGLRenderingContext::makeScreenshot() {
    return AImage();
}
