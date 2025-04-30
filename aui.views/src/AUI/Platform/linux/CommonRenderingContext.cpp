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
// Created by Alex2772 on 12/9/2021.
//

#include <AUI/Platform/CommonRenderingContext.h>
#include <AUI/Util/ARandom.h>
#include <X11/extensions/sync.h>
#include "AUI/GL/OpenGLRenderer.h"
#include "AUI/Util/kAUI.h"
#include "AUI/Logging/ALogger.h"
#include <AUI/UITestState.h>

aui::assert_not_used_when_null<Display*> PlatformAbstractionX11::ourDisplay = nullptr;
Screen* PlatformAbstractionX11::ourScreen = nullptr;
CommonRenderingContext::Atoms PlatformAbstractionX11::ourAtoms;


int xerrorhandler(Display* dsp, XErrorEvent* error) {
    if (PlatformAbstractionX11::ourDisplay == dsp) {
        char errorstring[0x100];
        XGetErrorText(dsp, error->error_code, errorstring, sizeof(errorstring));
        ALogger::info("X11") << "Error: " << errorstring << "\n" << AStacktrace::capture(2);
    }
    return 0;
}

void CommonRenderingContext::ensureXLibInitialized() {
    if (UITestState::isTesting()) {
        return;
    }
    struct DisplayInstance {

    public:
        DisplayInstance() {
            auto d = PlatformAbstractionX11::ourDisplay = XOpenDisplay(nullptr);
            if (d == nullptr) return;
            XSetErrorHandler(xerrorhandler);
            PlatformAbstractionX11::ourScreen = DefaultScreenOfDisplay(PlatformAbstractionX11::ourDisplay);

            PlatformAbstractionX11::ourAtoms.wmProtocols = XInternAtom(d, "WM_PROTOCOLS", False);
            PlatformAbstractionX11::ourAtoms.wmDeleteWindow = XInternAtom(d, "WM_DELETE_WINDOW", False);
            PlatformAbstractionX11::ourAtoms.wmHints = XInternAtom(d, "_MOTIF_WM_HINTS", true);
            PlatformAbstractionX11::ourAtoms.wmState = XInternAtom(d, "WM_STATE", true);
            PlatformAbstractionX11::ourAtoms.netWmState = XInternAtom(d, "_NET_WM_STATE", false);
            PlatformAbstractionX11::ourAtoms.netWmStateMaximizedVert = XInternAtom(d, "_NET_WM_STATE_MAXIMIZED_VERT", false);
            PlatformAbstractionX11::ourAtoms.netWmStateMaximizedHorz = XInternAtom(d, "_NET_WM_STATE_MAXIMIZED_HORZ", false);
            PlatformAbstractionX11::ourAtoms.clipboard = XInternAtom(d, "CLIPBOARD", False);
            PlatformAbstractionX11::ourAtoms.utf8String = XInternAtom(d, "UTF8_STRING", False);
            PlatformAbstractionX11::ourAtoms.textPlain = XInternAtom(d, "text/plain", False);
            PlatformAbstractionX11::ourAtoms.textPlainUtf8 = XInternAtom(d, "text/plain;charset=utf-8", False);
            PlatformAbstractionX11::ourAtoms.auiClipboard = XInternAtom(d, "AUI_CLIPBOARD", False);
            PlatformAbstractionX11::ourAtoms.incr = XInternAtom(d, "INCR", False);
            PlatformAbstractionX11::ourAtoms.targets = XInternAtom(d, "TARGETS", False);
            PlatformAbstractionX11::ourAtoms.netWmSyncRequest = XInternAtom(d, "_NET_WM_SYNC_REQUEST", False);
            PlatformAbstractionX11::ourAtoms.netWmSyncRequestCounter = XInternAtom(d, "_NET_WM_SYNC_REQUEST_COUNTER", False);
        }

        ~DisplayInstance() {
            //XCloseDisplay(ourDisplay);
            //XFree(ourScreen);

        }
    };
    static DisplayInstance display;
}

void CommonRenderingContext::initX11Window(const IRenderingContext::Init &init, XSetWindowAttributes& swa, XVisualInfo* vi) {
    do_once {
        if (!XSupportsLocale() || XSetLocaleModifiers("@im=none") == NULL) {
            throw AException("Your X server does not support locales.");
        }
    }
    auto& window = init.window;
    ensureXLibInitialized();

    static XIM im;
    static XIMStyles *styles;

    im = XOpenIM(ourDisplay, NULL, NULL, NULL);
    if (im == NULL) {
        throw AException("Could not open input method");
    }

    if (XGetIMValues(im, XNQueryInputStyle, &styles, NULL)) {
        throw AException("XIM Can't get styles");
    }

    window.mHandle = XCreateWindow(ourDisplay,
                                   ourScreen->root,
                                   0, 0,
                                   init.width, init.height,
                                   0,
                                   vi->depth,
                                   InputOutput,
                                   vi->visual,
                                   CWColormap | CWEventMask | CWCursor, &swa);

    // XSync
    {
        XSyncValue value;
        XSyncIntToValue(&value, 0);
        window.mXsyncRequestCounter.counter = XSyncCreateCounter(ourDisplay, value);
        XChangeProperty(ourDisplay,
                        window.mHandle,
                        ourAtoms.netWmSyncRequestCounter,
                        XA_CARDINAL,
                        32,
                        PropModeReplace,
                        (const unsigned char*)&window.mXsyncRequestCounter.counter, 1);

    }

    window.mIC = XCreateIC(im, XNInputStyle, XIMPreeditNothing | XIMStatusNothing, XNClientWindow, window.mHandle, NULL);
    if (window.mIC == NULL) {
        throw AException("Could not get IC");
    }
    XSetICFocus((XIC)window.mIC);


    auto title = init.name.toStdString();
    XStoreName(ourDisplay, window.mHandle, title.c_str());
    XChangeProperty(ourDisplay, window.mHandle, XInternAtom(ourDisplay, "_NET_WM_NAME", false),
                    XInternAtom(ourDisplay, "UTF8_STRING", false), 8, PropModeReplace,
                    reinterpret_cast<const unsigned char*>(title.c_str()), title.length());

    XSetWMProtocols(ourDisplay, window.mHandle, &ourAtoms.wmDeleteWindow, 1);

}

void CommonRenderingContext::init(const Init& init) {


    IRenderingContext::init(init);
}

void CommonRenderingContext::destroyNativeWindow(AWindowBase& window) {
    if (auto w = dynamic_cast<AWindow*>(&window)) {
        XDestroyWindow(ourDisplay, w->mHandle);
    }
}

void CommonRenderingContext::beginPaint(AWindowBase& window) {
}

void CommonRenderingContext::endPaint(AWindowBase& window) {
}
