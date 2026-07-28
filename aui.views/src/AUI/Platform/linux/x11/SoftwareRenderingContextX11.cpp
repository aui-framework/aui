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

#include "SoftwareRenderingContextX11.h"

static XVisualInfo* vi = nullptr;

SoftwareRenderingContextX11::~SoftwareRenderingContextX11() {
    if (mBitmapBlob && !mXImage) {
        free(mBitmapBlob);
        mBitmapBlob = nullptr;
    }
}

void SoftwareRenderingContextX11::init(const IRenderingContext::Init& init) {
    static XSetWindowAttributes swa;
    if (!vi) {
        XVisualInfo viTemplate;
        aui::zero(viTemplate);

        auto screen = DefaultScreen(PlatformAbstractionX11::ourDisplay);

        viTemplate.visualid = DefaultVisual(PlatformAbstractionX11::ourDisplay, screen)->visualid;
        viTemplate.c_class = TrueColor;

        int count;
        vi = XGetVisualInfo(PlatformAbstractionX11::ourDisplay, VisualIDMask | VisualClassMask, &viTemplate, &count);
        if (!vi) {
            throw AException("unable to pick visual info");
        }
        auto cmap = XCreateColormap(PlatformAbstractionX11::ourDisplay, PlatformAbstractionX11::ourScreen->root, vi->visual, AllocNone);
        swa.colormap = cmap;
        swa.event_mask =
            ExposureMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | StructureNotifyMask |
            PointerMotionMask | StructureNotifyMask | PropertyChangeMask | StructureNotifyMask;
    }
    xInitNativeWindow(init, swa, vi);
    mBitmapSize = init.window.getSize();
    XGCValues gcv {
        .graphics_exposures = false,
    };
    mGC = aui::ptr::manage_unique(
        XCreateGC(PlatformAbstractionX11::ourDisplay, PlatformAbstractionX11::nativeHandle(init.window), GCGraphicsExposures, &gcv),
        [](GC c) { XFreeGC(PlatformAbstractionX11::ourDisplay, c); });
    reallocate();
    if (!mXImage) {
        throw AException("unable to create XImage");
    }
    SoftwareRenderingContext::endResize(init.window);
}

void SoftwareRenderingContextX11::destroyNativeWindow(ASurface& window) {
    xDestroyNativeWindow(window);
}

void SoftwareRenderingContextX11::endPaint(ASurface& window) {
    SoftwareRenderingContext::endPaint(window);
    if (auto nativeWindow = dynamic_cast<AWindow*>(&window)) {
        XPutImage(PlatformAbstractionX11::ourDisplay,
                  nativeWindow->nativeHandle(),
                  mGC.get(),
                  mXImage.get(),
                  0, 0, // source x, y
                  0, 0, // dest   x, y
                  nativeWindow->getWidth(), nativeWindow->getHeight());
        XSync(PlatformAbstractionX11::ourDisplay, false);
    }
}

void SoftwareRenderingContextX11::reallocate() {
    mXImage.reset();
    SoftwareRenderingContext::reallocate();
    if (!PlatformAbstractionX11::ourDisplay.value() || !vi) {
        return;
    }
    mXImage = aui::ptr::manage_unique(
        XCreateImage(
            PlatformAbstractionX11::ourDisplay, vi->visual, vi->depth, ZPixmap, 0, reinterpret_cast<char *>(mBitmapBlob), mBitmapSize.x,
            mBitmapSize.y, 32, 0),
        [&](XImage *image) {
            mBitmapBlob = nullptr;   // XLib would deallocate memory for us
            XDestroyImage(image);
        });
}
