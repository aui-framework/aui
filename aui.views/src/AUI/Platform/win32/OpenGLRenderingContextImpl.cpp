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

#include <GL/wglew.h>
#include <AUI/GL/OpenGLRenderer.h>
#include <AUI/GL/State.h>
#include "AUI/Util/kAUI.h"


HGLRC OpenGLRenderingContext::ourHrc = nullptr;

static constexpr auto LOG_TAG = "OpenGL";

void OpenGLRenderingContext::makeCurrent(HDC hdc) noexcept {
    static HDC prev = nullptr;
    if (prev != hdc)
    {
        prev = hdc;
        if (hdc != nullptr) {
            bool ok = wglMakeCurrent(hdc, ourHrc);
            AUI_ASSERT(ok);
        }
    }
}


void OpenGLRenderingContext::init(const Init& init) {
    CommonRenderingContext::init(init);

    // INITIALIZE OPENGL
    static PIXELFORMATDESCRIPTOR pfd;
    static int pxf;
    if (ourHrc == nullptr) {
        ALogger::info(LOG_TAG) << ("Creating context...");
        struct FakeWindow {
            HWND mHwnd;
            HDC mDC;

            explicit FakeWindow(HWND hwnd) : mHwnd(hwnd) {
                mDC = GetDC(mHwnd);
            }

            ~FakeWindow() {
                ReleaseDC(mHwnd, mDC);
                DestroyWindow(mHwnd);
            }
        } fakeWindow(CreateWindowEx(WS_EX_DLGMODALFRAME, aui::win32::toWchar(mWindowClass), aui::win32::toWchar(init.name), WS_OVERLAPPEDWINDOW,
                                    GetSystemMetrics(SM_CXSCREEN) / 2 - init.width / 2,
                                    GetSystemMetrics(SM_CYSCREEN) / 2 - init.height / 2, init.width, init.height,
                                    init.parent != nullptr ? init.parent->mHandle : nullptr, nullptr, GetModuleHandle(nullptr), nullptr));

        memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
        pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
        pfd.nVersion = 1;
        pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_SUPPORT_COMPOSITION;
        pfd.iPixelType = PFD_TYPE_RGBA;
        pfd.cColorBits = 8;
        pfd.cStencilBits = 8;
        size_t iPixelFormat = ChoosePixelFormat(fakeWindow.mDC, &pfd);
        DescribePixelFormat(fakeWindow.mDC, iPixelFormat, sizeof(pfd), &pfd);
        SetPixelFormat(fakeWindow.mDC, iPixelFormat, &pfd);

        // context initialization
        ourHrc = wglCreateContext(fakeWindow.mDC);
        makeCurrent(fakeWindow.mDC);

        ALogger::info(LOG_TAG) << ("Initialized temporary context");

        if (!glewExperimental) {
            glewExperimental = true;
            if (glewInit() != GLEW_OK) {
                AMessageBox::show(nullptr, "OpenGL", "Could not initialize context");
                throw AException("glewInit failed");
            }
        }
        bool k;

        auto makeContext = [&](unsigned i) {
            const int iPixelFormatAttribList[] =
                    {
                            WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
                            WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
                            WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
                            WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
                            WGL_COLOR_BITS_ARB, 24,
                            WGL_ALPHA_BITS_ARB, 8,
                            WGL_DEPTH_BITS_ARB, 24,
                            WGL_STENCIL_BITS_ARB, 8,
                            //WGL_SAMPLE_BUFFERS_ARB, GL_TRUE,
                            //WGL_SAMPLES_ARB, i,
                            0
                    };

            UINT iNumFormats;

            wglChoosePixelFormatARB(mWindowDC, iPixelFormatAttribList, nullptr, 1, &pxf, &iNumFormats);
            AUI_ASSERT(iNumFormats);
            DescribePixelFormat(mWindowDC, pxf, sizeof(pfd), &pfd);
            k = SetPixelFormat(mWindowDC, pxf, &pfd);
        };
        makeContext(16);
        if (!k) {
            ALogger::info(LOG_TAG) << ("Could not set pixel format with MSAA; trying to do the same but without MSAA");
            makeContext(0);
            if (!k) {
                ALogger::info(LOG_TAG) << ("Could not set pixel format even without MSAA. Giving up.");
                throw AException("Could set pixel format");
            } else {
                ALogger::info(LOG_TAG) << ("Successfully set pixel format without MSAA");
            }
        } else {
            ALogger::info(LOG_TAG) << ("Successfully set pixel format with MSAA");
        }
        GLint attribs[] =
                {
                        WGL_CONTEXT_MAJOR_VERSION_ARB, mConfig.majorVersion,
                        WGL_CONTEXT_MINOR_VERSION_ARB, mConfig.minorVersion,
                        WGL_CONTEXT_PROFILE_MASK_ARB, mConfig.profile == ARenderingContextOptions::OpenGL::Profile::CORE
                                                        ? WGL_CONTEXT_CORE_PROFILE_BIT_ARB : WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
                        0
                };
        {
            HGLRC compHrc = wglCreateContextAttribsARB(mWindowDC, nullptr, attribs);
            if (compHrc) {
                auto prevContext = ourHrc;
                ourHrc = compHrc;
                makeCurrent(mWindowDC);
                wglDeleteContext(prevContext);
            } else
                throw AException("Failed to create OpenGL {}.{} {} context"_format(mConfig.majorVersion, mConfig.minorVersion, mConfig.profile));
        }
        ALogger::info(LOG_TAG) << ("Context is ready");


        //makeCurrent(mDC);
    } else {
        bool k = SetPixelFormat(mWindowDC, pxf, &pfd);
        AUI_ASSERT(k);
    }

    mRenderer = ourRenderer();
    makeCurrent(mWindowDC);
    // vsync
    wglSwapIntervalEXT(!(ARenderingContextOptions::get().flags & ARenderContextFlags::NO_VSYNC));

    GLint stencilBits = 0;
    glGetIntegerv(GL_STENCIL_BITS, &stencilBits);
}

void OpenGLRenderingContext::destroyNativeWindow(ABaseWindow& window) {
    CommonRenderingContext::destroyNativeWindow(window);
    makeCurrent(nullptr);
}

void OpenGLRenderingContext::beginPaint(ABaseWindow& window) {
    CommonRenderingContext::beginPaint(window);

    makeCurrent(mSmoothResize ? mPainterDC : mWindowDC);
    beginFramebuffer(window.getSize());
    mRenderer->beginPaint(window.getSize());
}

void OpenGLRenderingContext::beginResize(ABaseWindow& window) {
    makeCurrent(mWindowDC);
}

void OpenGLRenderingContext::endResize(ABaseWindow& window) {

}

void OpenGLRenderingContext::endPaint(ABaseWindow& window) {
    endFramebuffer();
    mRenderer->endPaint();
    SwapBuffers(mSmoothResize ? mPainterDC : mWindowDC);
    if (mSmoothResize) {
        makeCurrent(nullptr);
    }
    //makeCurrent(mWindowDC);
    CommonRenderingContext::endPaint(window);
}

OpenGLRenderingContext::~OpenGLRenderingContext() {
}

AImage OpenGLRenderingContext::makeScreenshot() {
    return AImage();
}
