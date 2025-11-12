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

#pragma once

#include <AUI/Platform/IRenderingContext.h>
#include <AUI/Platform/linux/x11/PlatformAbstractionX11.h>

class RenderingContextX11 {
public:
    virtual ~RenderingContextX11() = default;


    /**
     * _NET_WM_SYNC_REQUEST (resize flicker fix) update request counter
     */
    struct Sync {
        uint32_t lo = 0;
        uint32_t hi = 0;
        XID counter;
    };

    Sync& sync() { return mXsyncRequestCounter; }

    XIC ic() { return mIC; }

protected:
    void xInitNativeWindow(const IRenderingContext::Init& init, XSetWindowAttributes& swa, XVisualInfo* vi);
    void xDestroyNativeWindow(ASurface& window);
    Sync mXsyncRequestCounter;
    XIC mIC = nullptr;
};
