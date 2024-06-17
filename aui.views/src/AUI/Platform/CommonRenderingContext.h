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

#pragma once

#include "IRenderingContext.h"
#include <AUI/Platform/AWindow.h>
#include "AUI/Traits/values.h"
#include "ARenderingContextOptions.h"

#if AUI_PLATFORM_LINUX
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <X11/Xatom.h>
#endif

class CommonRenderingContext: public IRenderingContext {
public:
#if AUI_PLATFORM_LINUX
    static void ensureXLibInitialized();
    static aui::assert_not_used_when_null<Display*> ourDisplay;
    static Screen* ourScreen;

    static struct Atoms {
        Atom wmProtocols;
        Atom wmDeleteWindow;
        Atom wmHints;
        Atom wmState;
        Atom netWmState;
        Atom netWmStateMaximizedVert;
        Atom netWmStateMaximizedHorz;
        Atom clipboard;
        Atom utf8String;
        Atom textPlain;
        Atom textPlainUtf8;
        Atom auiClipboard;
        Atom incr;
        Atom targets;
        Atom netWmSyncRequest;
        Atom netWmSyncRequestCounter;
    } ourAtoms;
#endif
#if AUI_PLATFORM_MACOS
    void requestFrame();
#endif

    void init(const Init& init) override;

    void beginPaint(ABaseWindow& window) override;

    void endPaint(ABaseWindow& window) override;

    ~CommonRenderingContext() override = default;

    void destroyNativeWindow(ABaseWindow& window) override;

protected:
#if AUI_PLATFORM_WIN

    AString mWindowClass;

    /**
     * GetDC() HDC
     */
    HDC mWindowDC;

    /**
     * BeginPaint() HDC
     */
    HDC mPainterDC = nullptr;

    PAINTSTRUCT mPaintstruct;
#endif
#if AUI_PLATFORM_LINUX
    void initX11Window(const Init& init, XSetWindowAttributes& swa, XVisualInfo* vi);
#endif
#if AUI_PLATFORM_MACOS
    AWindow* mWindow;
    void* mDisplayLink;
    bool mFrameScheduled = false;
#endif

    bool mSmoothResize = !(ARenderingContextOptions::get().flags & ARenderContextFlags::NO_SMOOTH);
};