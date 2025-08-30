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

#include "RenderingContextX11.h"
#include <AUI/Util/ARandom.h>
#include "AUI/GL/OpenGLRenderer.h"
#include "AUI/Util/kAUI.h"
#include "AUI/Logging/ALogger.h"
#include <AUI/UITestState.h>

void RenderingContextX11::xInitNativeWindow(
    const IRenderingContext::Init& init, XSetWindowAttributes& swa, XVisualInfo* vi) {
    AUI_DO_ONCE {
        if (!XSupportsLocale() || XSetLocaleModifiers("@im=none") == nullptr) {
            throw AException("Your X server does not support locales.");
        }
    }
    auto& window = init.window;
    PlatformAbstractionX11::ensureXLibInitialized();

    static XIM im = nullptr;
    static XIMStyles* styles;

    if (im == nullptr) {
        im = XOpenIM(PlatformAbstractionX11::ourDisplay, nullptr, nullptr, nullptr);
        if (im == nullptr) {
            throw AException("Could not open input method");
        }
        if (XGetIMValues(im, XNQueryInputStyle, &styles, nullptr)) {
            throw AException("XIM Can't get styles");
        }
    }

    auto handle = XCreateWindow(
        PlatformAbstractionX11::ourDisplay, PlatformAbstractionX11::ourScreen->root, 0, 0, init.width, init.height, 0,
        vi->depth, InputOutput, vi->visual, CWColormap | CWEventMask | CWCursor, &swa);
    if (handle == 0) {
        throw AException("Could not create window");
    }

    PlatformAbstractionX11::setNativeHandle(window, handle);

    // XSync
    {
        XSyncValue value;
        XSyncIntToValue(&value, 0);
        mXsyncRequestCounter.counter = XSyncCreateCounter(PlatformAbstractionX11::ourDisplay, value);
        XChangeProperty(
            PlatformAbstractionX11::ourDisplay, handle, PlatformAbstractionX11::ourAtoms.netWmSyncRequestCounter, XA_CARDINAL, 32, PropModeReplace,
            (const unsigned char*) &mXsyncRequestCounter.counter, 1);
    }

    mIC = XCreateIC(im, XNInputStyle, XIMPreeditNothing | XIMStatusNothing, XNClientWindow, handle, nullptr);
    if (mIC == nullptr) {
        throw AException("Could not get IC");
    }
    XSetICFocus(mIC);

    auto title = init.name.toStdString();
    XStoreName(PlatformAbstractionX11::ourDisplay, handle, title.c_str());
    XChangeProperty(
        PlatformAbstractionX11::ourDisplay, handle, XInternAtom(PlatformAbstractionX11::ourDisplay, "_NET_WM_NAME", false),
        XInternAtom(PlatformAbstractionX11::ourDisplay, "UTF8_STRING", false), 8, PropModeReplace,
        reinterpret_cast<const unsigned char*>(title.c_str()), title.length());

    XSetWMProtocols(PlatformAbstractionX11::ourDisplay, handle, &PlatformAbstractionX11::ourAtoms.wmDeleteWindow, 1);
    init.window.updateDpi();
}

void RenderingContextX11::xDestroyNativeWindow(AWindowBase& window) {
    XDestroyIC(mIC);
    if (auto w = dynamic_cast<AWindow*>(&window)) {
        XDestroyWindow(PlatformAbstractionX11::ourDisplay, PlatformAbstractionX11::nativeHandle(*w));
    }
}
