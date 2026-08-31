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

#include "PlatformAbstractionX11.h"
namespace {
std::string gClipboardText;

AWindow* getTopLevelWindow() {
    auto basicWindow = AWindow::current();
    auto auiWindow = dynamic_cast<AWindow*>(basicWindow);
    if (auiWindow) {
        while (auto p = dynamic_cast<AWindow*>(auiWindow->getParent())) {
            auiWindow = p;
        }
        if (!(auiWindow->windowStyle() & WindowStyle::SYS)) {
            return auiWindow;
        }
    }
    for (const auto& w : AWindow::getWindowManager().getWindows()) {
        if (w && !(w->windowStyle() & WindowStyle::SYS)) {
            return w.get();
        }
    }
    if (auiWindow) {
        return auiWindow;
    }
    if (!AWindow::getWindowManager().getWindows().empty()) {
        return AWindow::getWindowManager().getWindows().front().get();
    }
    return nullptr;
}
}

void PlatformAbstractionX11::setClipboardText(const AString& text) {
    auto auiWindow = getTopLevelWindow();
    if (!auiWindow)
        return;
    gClipboardText = text.toStdString();
    auto handle = nativeHandle(*auiWindow);
    XSetSelectionOwner(
        PlatformAbstractionX11::ourDisplay, PlatformAbstractionX11::ourAtoms.clipboard, handle,
        CurrentTime);
    XSetSelectionOwner(
        PlatformAbstractionX11::ourDisplay, XA_PRIMARY, handle,
        CurrentTime);
    XFlush(PlatformAbstractionX11::ourDisplay);
}

AString PlatformAbstractionX11::getClipboardText() {
    Atom requestedSelection = ourAtoms.clipboard;
    auto owner = XGetSelectionOwner(PlatformAbstractionX11::ourDisplay, requestedSelection);
    if (owner == None)
    {
        requestedSelection = XA_PRIMARY;
        owner = XGetSelectionOwner(PlatformAbstractionX11::ourDisplay, requestedSelection);
    }
    if (owner == None)
    {
        return {};
    }
    auto auiWindow = getTopLevelWindow();
    if (!auiWindow)
        return {};
    auto nativeHandle = auiWindow->getNativeHandle();
    AUI_ASSERT(nativeHandle);

    if (owner == nativeHandle) {
        return AString::fromUtf8(gClipboardText);
    }

    XConvertSelection(PlatformAbstractionX11::ourDisplay, requestedSelection, PlatformAbstractionX11::ourAtoms.utf8String, PlatformAbstractionX11::ourAtoms.auiClipboard, nativeHandle,
                      CurrentTime);
    XFlush(PlatformAbstractionX11::ourDisplay);
    XEvent ev;
    for (int i = 0; i < 30; ++i)
    {
        XNextEvent(PlatformAbstractionX11::ourDisplay, &ev);
        switch (ev.type)
        {
            case SelectionNotify: {
                if (ev.xselection.property == None) {
                    return {};
                }
                Atom da, incr, type;
                int di;
                unsigned long size, dul;
                unsigned char *prop_ret = NULL;

                XGetWindowProperty(PlatformAbstractionX11::ourDisplay, nativeHandle, PlatformAbstractionX11::ourAtoms.auiClipboard, 0, 0, False, AnyPropertyType,
                                   &type, &di, &dul, &size, &prop_ret);
                XFree(prop_ret);

                if (type == PlatformAbstractionX11::ourAtoms.incr)
                {
                    ALogger::warn("Clipboard data is too large and INCR mechanism not implemented");
                    return {};
                }

                XGetWindowProperty(PlatformAbstractionX11::ourDisplay, nativeHandle, PlatformAbstractionX11::ourAtoms.auiClipboard, 0, size, False, AnyPropertyType,
                                   &da, &di, &dul, &dul, &prop_ret);
                AString clipboardData = reinterpret_cast<const char*>(prop_ret);
                XFree(prop_ret);

                XDeleteProperty(PlatformAbstractionX11::ourDisplay, nativeHandle, PlatformAbstractionX11::ourAtoms.auiClipboard);
                return clipboardData;
            }
            default:
                auiWindow->getThread() << [this, ev] {
                  xProcessEvent(const_cast<XEvent&>(ev));
                };
        }
    }
    return "";
}

void PlatformAbstractionX11::xClipboardClear() {
    gClipboardText.clear();
}

void PlatformAbstractionX11::xHandleClipboard(const XEvent& ev) {
    if (ev.xselectionrequest.property == None) {
        return;
    }

    Atom target = ev.xselectionrequest.target;
    Atom property = ev.xselectionrequest.property;
    Atom stringAtom = XInternAtom(ourDisplay, "STRING", False);
    Atom textAtom = XInternAtom(ourDisplay, "TEXT", False);

    XSelectionEvent ssev = { 0 };
    ssev.type = SelectionNotify;
    ssev.display = ourDisplay;
    ssev.requestor = ev.xselectionrequest.requestor;
    ssev.selection = ev.xselectionrequest.selection;
    ssev.target = target;
    ssev.property = property;
    ssev.time = ev.xselectionrequest.time;

    if (target == ourAtoms.utf8String ||
        target == ourAtoms.textPlain ||
        target == ourAtoms.textPlainUtf8 ||
        target == textAtom) {
        Atom responseType = target;
        if (target == textAtom) {
            responseType = ourAtoms.utf8String;
        }
        XChangeProperty(ourDisplay,
                        ev.xselectionrequest.requestor,
                        property,
                        responseType,
                        8,
                        PropModeReplace,
                        reinterpret_cast<const unsigned char*>(gClipboardText.data()),
                        static_cast<int>(gClipboardText.size()));
    } else if (target == stringAtom || target == XA_STRING) {
        std::string latin1;
        for (char32_t c : AString::fromUtf8(gClipboardText).toUtf32()) {
            if (c <= 0xff) {
                latin1 += static_cast<char>(c);
            } else {
                latin1 += '?';
            }
        }
        XChangeProperty(ourDisplay,
                        ev.xselectionrequest.requestor,
                        property,
                        target,
                        8,
                        PropModeReplace,
                        reinterpret_cast<const unsigned char*>(latin1.data()),
                        static_cast<int>(latin1.size()));
    } else if (target == ourAtoms.targets) {
        Atom atoms[] = {
            ourAtoms.targets,
            ourAtoms.utf8String,
            ourAtoms.textPlain,
            ourAtoms.textPlainUtf8,
            textAtom,
            stringAtom,
            XA_STRING,
        };
        XChangeProperty(ourDisplay,
                        ev.xselectionrequest.requestor,
                        property,
                        XA_ATOM,
                        32,
                        PropModeReplace,
                        reinterpret_cast<const unsigned char*>(atoms),
                        static_cast<int>(std::size(atoms)));
    } else {
        ssev.property = None;
    }

    XSendEvent(ourDisplay, ev.xselectionrequest.requestor, False, 0, (XEvent *)&ssev);
    XFlush(ourDisplay);
}
