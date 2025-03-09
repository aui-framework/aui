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
typedef GLXContext (*PFNGLXCREATECONTEXTATTRIBSARBPROC)(
    Display* dpy, GLXFBConfig config, GLXContext share_context, Bool direct, const int* attrib_list);
static PFNGLXCREATECONTEXTATTRIBSARBPROC glXCreateContextAttribsARB = nullptr;

typedef void (*PFNGLXSWAPINTERVALEXTPROC)(Display* dpy, GLXDrawable drawable, int interval);
static PFNGLXSWAPINTERVALEXTPROC glXSwapIntervalEXT = nullptr;

GLXContext OpenGLRenderingContext::ourContext = nullptr;

OpenGLRenderingContext::~OpenGLRenderingContext() {}

void OpenGLRenderingContext::init(const Init& init) {
    CommonRenderingContext::init(init);
    static XSetWindowAttributes swa;
    static XVisualInfo* vi;
    if (ourContext == nullptr) {
        glXCreateContextAttribsARB = reinterpret_cast<PFNGLXCREATECONTEXTATTRIBSARBPROC>(
            glXGetProcAddress(reinterpret_cast<const GLubyte*>("glXCreateContextAttribsARB")));

        GLint att[] = {
            GLX_X_RENDERABLE,
            True,   // 1
            GLX_DRAWABLE_TYPE,
            GLX_WINDOW_BIT,   // 3
            GLX_RENDER_TYPE,
            GLX_RGBA_BIT,   // 5
            GLX_X_VISUAL_TYPE,
            GLX_TRUE_COLOR,   // 7
            GLX_RED_SIZE,
            8,   // 9
            GLX_GREEN_SIZE,
            8,   // 11
            GLX_BLUE_SIZE,
            8,   // 13
            GLX_ALPHA_SIZE,
            8,   // 15
            GLX_DEPTH_SIZE,
            24,
            GLX_STENCIL_SIZE,
            8,
            GLX_DOUBLEBUFFER,
            true,
            GLX_STENCIL_SIZE,
            8,
            GLX_SAMPLE_BUFFERS,
            0,
            GLX_SAMPLES,
            0,
            None
        };

        int fbcount;
        GLXFBConfig* fbc = glXChooseFBConfig(ourDisplay, DefaultScreen(ourDisplay), att, &fbcount);

        if (fbc == nullptr || fbcount <= 0) {
            // try to reduce system requirements
            size_t indexToReduce = std::size(att) - 2;
            do {
                ALogger::warn("[OpenGL compatibility] Reduced OpenGL requirements: pass {}"_format(
                    (std::size(att) - indexToReduce) / 2 - 1));
                att[indexToReduce] = 0;
                indexToReduce -= 2;
                fbc = glXChooseFBConfig(ourDisplay, DefaultScreen(ourDisplay), att, &fbcount);
            } while ((fbc == nullptr || fbcount <= 0) && indexToReduce > 13);   // up to GLX_BLUE_SIZE

            if (fbc == nullptr || fbcount <= 0) {
                // try to disable rgba.
                att[5] = 0;
                ALogger::warn("[OpenGL compatibility] Disabled RGBA");
                fbc = glXChooseFBConfig(ourDisplay, DefaultScreen(ourDisplay), att, &fbcount);

                if (fbc == nullptr || fbcount <= 0) {
                    // use default attribs
                    ALogger::warn("[OpenGL compatibility] Using default attribs");
                    glXChooseFBConfig(ourDisplay, DefaultScreen(ourDisplay), nullptr, &fbcount);
                    if (fbc == nullptr || fbcount <= 0) {
                        // giving up.
                        ALogger::err("[OpenGL compatibility] System hardware is not supported. Giving up.");
                        exit(-1);
                    }
                }
            }
        }

        int best_fbc = 0;

        for (int i = 0; i < fbcount; ++i) {
            vi = glXGetVisualFromFBConfig(ourDisplay, fbc[i]);
            if (vi) {
                int samples;
                glXGetFBConfigAttrib(ourDisplay, fbc[i], GLX_SAMPLES, &samples);
                if (samples == 0) {
                    best_fbc = i;
                    break;
                }
            }
            XFree(vi);
        }

        GLXFBConfig bestFbc = fbc[best_fbc];

        // Be sure to free the FBConfig list allocated by glXChooseFBConfig()
        XFree(fbc);

        // Get a visual
        vi = glXGetVisualFromFBConfig(ourDisplay, bestFbc);
        auto cmap = XCreateColormap(ourDisplay, ourScreen->root, vi->visual, AllocNone);
        swa.colormap = cmap;
        swa.event_mask =
            ExposureMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | StructureNotifyMask |
            PointerMotionMask | StructureNotifyMask | PropertyChangeMask | StructureNotifyMask;
        if (glXCreateContextAttribsARB) {
            int attribList[] = {
                GLX_CONTEXT_MAJOR_VERSION_ARB,
                3,
                GLX_CONTEXT_MINOR_VERSION_ARB,
                3,
                GLX_CONTEXT_PROFILE_MASK_ARB,
                GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
                None,
            };
            ourContext = glXCreateContextAttribsARB(ourDisplay, bestFbc, nullptr, true, attribList);
        } else {
            ALogger::warn("OpenGLRenderingContext") << "glXCreateContextAttribsARB is not available";
            ourContext = glXCreateContext(ourDisplay, vi, nullptr, true);
        }
    }
    initX11Window(init, swa, vi);
    glXMakeCurrent(ourDisplay, init.window.mHandle, ourContext);

    if (!glewExperimental) {
        glewExperimental = true;
        if (glewInit() != GLEW_OK) {
            throw AException("glewInit failed");
        }
        ALogger::info("OpenGL context is ready");
    }

    if (init.parent) {
        XSetTransientForHint(ourDisplay, init.window.mHandle, init.parent->mHandle);
    }

    // vsync
    do_once {
        glXSwapIntervalEXT = reinterpret_cast<PFNGLXSWAPINTERVALEXTPROC>(
            glXGetProcAddress(reinterpret_cast<const GLubyte*>("glXSwapIntervalEXT")));
    }

    if (glXSwapIntervalEXT) {
        glXSwapIntervalEXT(
            ourDisplay, init.window.mHandle, !(ARenderingContextOptions::get().flags & ARenderContextFlags::NO_VSYNC));
    }

    mRenderer = ourRenderer();
}

void OpenGLRenderingContext::destroyNativeWindow(AWindowBase& window) {
    CommonRenderingContext::destroyNativeWindow(window);
    if (auto w = dynamic_cast<AWindow*>(&window)) {
        XDestroyWindow(ourDisplay, w->mHandle);
    }
}

void OpenGLRenderingContext::beginPaint(AWindowBase& window) {
    CommonRenderingContext::beginPaint(window);

    if (auto w = dynamic_cast<AWindow*>(&window)) {
        glXMakeCurrent(ourDisplay, w->mHandle, ourContext);
    }
    beginFramebuffer(window.getSize());
    mRenderer->beginPaint(window.getSize());
}

void OpenGLRenderingContext::endPaint(AWindowBase& window) {
    CommonRenderingContext::endPaint(window);
    endFramebuffer();
    mRenderer->endPaint();
    if (auto w = dynamic_cast<AWindow*>(&window)) {
        glXSwapBuffers(ourDisplay, w->mHandle);
    }
}

void OpenGLRenderingContext::beginResize(AWindowBase& window) {
    if (auto w = dynamic_cast<AWindow*>(&window)) {
        glXMakeCurrent(ourDisplay, w->mHandle, ourContext);
    }
}

void OpenGLRenderingContext::endResize(AWindowBase& window) {

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
