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
