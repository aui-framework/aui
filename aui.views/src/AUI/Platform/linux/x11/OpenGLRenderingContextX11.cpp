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

#include "OpenGLRenderingContextX11.h"

/* Typedef for the GL 3.0 context creation function */
typedef GLXContext (*PFNGLXCREATECONTEXTATTRIBSARBPROC)(
    Display* dpy, GLXFBConfig config, GLXContext share_context, Bool direct, const int* attrib_list);
static PFNGLXCREATECONTEXTATTRIBSARBPROC glXCreateContextAttribsARB = nullptr;

typedef void (*PFNGLXSWAPINTERVALEXTPROC)(Display* dpy, GLXDrawable drawable, int interval);
static PFNGLXSWAPINTERVALEXTPROC glXSwapIntervalEXT = nullptr;

GLXContext OpenGLRenderingContextX11::ourContext = nullptr;

void OpenGLRenderingContextX11::init(const IRenderingContext::Init& init) {
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
        GLXFBConfig* fbc = glXChooseFBConfig(
            PlatformAbstractionX11::ourDisplay, DefaultScreen(PlatformAbstractionX11::ourDisplay), att, &fbcount);

        if (fbc == nullptr || fbcount <= 0) {
            // try to reduce system requirements
            size_t indexToReduce = std::size(att) - 2;
            do {
                ALogger::warn("[OpenGL compatibility] Reduced OpenGL requirements: pass {}"_format(
                    (std::size(att) - indexToReduce) / 2 - 1));
                att[indexToReduce] = 0;
                indexToReduce -= 2;
                fbc = glXChooseFBConfig(
                    PlatformAbstractionX11::ourDisplay, DefaultScreen(PlatformAbstractionX11::ourDisplay), att,
                    &fbcount);
            } while ((fbc == nullptr || fbcount <= 0) && indexToReduce > 13);   // up to GLX_BLUE_SIZE

            if (fbc == nullptr || fbcount <= 0) {
                // try to disable rgba.
                att[5] = 0;
                ALogger::warn("[OpenGL compatibility] Disabled RGBA");
                fbc = glXChooseFBConfig(
                    PlatformAbstractionX11::ourDisplay, DefaultScreen(PlatformAbstractionX11::ourDisplay), att,
                    &fbcount);

                if (fbc == nullptr || fbcount <= 0) {
                    // use default attribs
                    ALogger::warn("[OpenGL compatibility] Using default attribs");
                    glXChooseFBConfig(
                        PlatformAbstractionX11::ourDisplay, DefaultScreen(PlatformAbstractionX11::ourDisplay), nullptr,
                        &fbcount);
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
            vi = glXGetVisualFromFBConfig(PlatformAbstractionX11::ourDisplay, fbc[i]);
            if (vi) {
                int samples;
                glXGetFBConfigAttrib(PlatformAbstractionX11::ourDisplay, fbc[i], GLX_SAMPLES, &samples);
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
        vi = glXGetVisualFromFBConfig(PlatformAbstractionX11::ourDisplay, bestFbc);
        auto cmap = XCreateColormap(
            PlatformAbstractionX11::ourDisplay, PlatformAbstractionX11::ourScreen->root, vi->visual, AllocNone);
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
            ourContext =
                glXCreateContextAttribsARB(PlatformAbstractionX11::ourDisplay, bestFbc, nullptr, true, attribList);
        } else {
            ALogger::warn("OpenGLRenderingContext") << "glXCreateContextAttribsARB is not available";
            ourContext = glXCreateContext(PlatformAbstractionX11::ourDisplay, vi, nullptr, true);
        }
    }
    xInitNativeWindow(init, swa, vi);
    glXMakeCurrent(PlatformAbstractionX11::ourDisplay, PlatformAbstractionX11::nativeHandle(init.window), ourContext);

    if (!glewExperimental) {
        glewExperimental = true;
        if (auto s = glewInit(); s != GLEW_OK) {
            throw AException("glewInit failed");
        }
        ALogger::info("OpenGL context is ready");
    }

    if (init.parent) {
        XSetTransientForHint(
            PlatformAbstractionX11::ourDisplay, PlatformAbstractionX11::nativeHandle(init.window),
            PlatformAbstractionX11::nativeHandle(*init.parent));
    }

    // vsync
    AUI_DO_ONCE {
        glXSwapIntervalEXT = reinterpret_cast<PFNGLXSWAPINTERVALEXTPROC>(
            glXGetProcAddress(reinterpret_cast<const GLubyte*>("glXSwapIntervalEXT")));
    }

    if (glXSwapIntervalEXT) {
        glXSwapIntervalEXT(
            PlatformAbstractionX11::ourDisplay, PlatformAbstractionX11::nativeHandle(init.window),
            !(ARenderingContextOptions::get().flags & ARenderContextFlags::NO_VSYNC));
    }

    mRenderer = ourRenderer();
}

void OpenGLRenderingContextX11::destroyNativeWindow(AWindowBase& window) { xDestroyNativeWindow(window); }

void OpenGLRenderingContextX11::beginPaint(AWindowBase& window) {
    // order is intentional
    if (auto w = dynamic_cast<AWindow*>(&window)) {
        glXMakeCurrent(PlatformAbstractionX11::ourDisplay, PlatformAbstractionX11::nativeHandle(*w), ourContext);
    }
    OpenGLRenderingContext::beginPaint(window);
}

void OpenGLRenderingContextX11::endPaint(AWindowBase& window) {
    OpenGLRenderingContext::endPaint(window);
    if (auto w = dynamic_cast<AWindow*>(&window)) {
        glXSwapBuffers(PlatformAbstractionX11::ourDisplay, PlatformAbstractionX11::nativeHandle(*w));
    }
}

void OpenGLRenderingContextX11::beginResize(AWindowBase& window) {
    OpenGLRenderingContext::beginResize(window);
    if (auto w = dynamic_cast<AWindow*>(&window)) {
        glXMakeCurrent(PlatformAbstractionX11::ourDisplay, PlatformAbstractionX11::nativeHandle(*w), ourContext);
    }
}
