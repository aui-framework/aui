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

#include "AUI/Platform/ACustomWindow.h"
#include "AUI/Platform/ADesktop.h"
#include "AUI/Platform/CommonRenderingContext.h"
#include "AUI/Platform/linux/x11/PlatformAbstractionX11.h"
#include <cstring>
#include <AUI/View/AButton.h>

#include <X11/Xlib.h>
#include <X11/Xatom.h>

ACustomWindow::ACustomWindow(const AString& name, AMetric width, AMetric height, AWindow* parent) :
        AWindow(name, width, height, parent) {
    setWindowStyle(WindowStyle::NO_TITLEBAR);
}

void ACustomWindow::onPointerPressed(const APointerPressedEvent& event) {
    if (event.position.y < mTitleHeight && event.asButton == AInput::LBUTTON) {
        if (isCaptionAt(event.position)) {
            // Hand the drag off to the window manager via the EWMH _NET_WM_MOVERESIZE
            // client message. This gives us native Aero-snap-like behaviour on KWin/Mutter
            // (edge tiling, half-screen) while we stay in charge of the rest of the window.
            Display* display = PlatformAbstractionX11::ourDisplay;
            if (display && mHandle) {
                static Atom atomMoveResize = XInternAtom(display, "_NET_WM_MOVERESIZE", False);
                const auto rootPos = ADesktop::getMousePosition();

                XUngrabPointer(display, CurrentTime);
                XFlush(display);

                XClientMessageEvent xclient;
                std::memset(&xclient, 0, sizeof(xclient));
                xclient.type = ClientMessage;
                xclient.window = static_cast<Window>(mHandle);
                xclient.message_type = atomMoveResize;
                xclient.format = 32;
                xclient.data.l[0] = rootPos.x;
                xclient.data.l[1] = rootPos.y;
                xclient.data.l[2] = 8;                       // _NET_WM_MOVERESIZE_MOVE
                xclient.data.l[3] = Button1;                 // button that triggered the drag
                xclient.data.l[4] = 0;                       // source indication: normal app
                XSendEvent(display,
                           XRootWindow(display, DefaultScreen(display)),
                           False,
                           SubstructureRedirectMask | SubstructureNotifyMask,
                           reinterpret_cast<XEvent*>(&xclient));
            }

            mDragging = true;
            mDragPos = event.position;
            emit dragBegin(event.position);
        }
    }
    AWindow::onPointerPressed(event);
}


void ACustomWindow::onPointerReleased(const APointerReleasedEvent& event) {
    AViewContainer::onPointerReleased(event);
}


bool ACustomWindow::isCaptionAt(const glm::ivec2& pos) {
    if (pos.y <= mTitleHeight) {
        if (auto v = getViewAtRecursive(pos)) {
            if (!(_cast<AButton>(v)) &&
                !v->getAssNames().contains(".override-title-dragging")) {
                return true;
            }
        }
    }
    return false;
}
void ACustomWindow::setSize(glm::ivec2 size) { AWindow::setSize(size); }
