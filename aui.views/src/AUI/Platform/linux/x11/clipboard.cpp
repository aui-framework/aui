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
std::string gPrimaryText;
Time gClipboardTimestamp = CurrentTime;
Time gPrimaryTimestamp = CurrentTime;

Time getServerTime(Display* dpy, Window win, Atom prop) {
    unsigned char dummy = 0;
    XChangeProperty(dpy, win, prop, XA_INTEGER, 8, PropModeReplace, &dummy, 1);
    XEvent ev;
    XIfEvent(dpy, &ev, [](Display*, XEvent* event, XPointer arg) -> Bool {
        return event->type == PropertyNotify && event->xproperty.atom == reinterpret_cast<Atom>(arg);
    }, reinterpret_cast<XPointer>(prop));
    return ev.xproperty.time;
}
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
    gPrimaryText = text.toStdString();
    auto handle = nativeHandle(*auiWindow);
    Time time = getServerTime(ourDisplay, handle, ourAtoms.auiClipboard);
    gClipboardTimestamp = time;
    gPrimaryTimestamp = time;
    XSetSelectionOwner(
        PlatformAbstractionX11::ourDisplay, PlatformAbstractionX11::ourAtoms.clipboard, handle,
        time);
    XSetSelectionOwner(
        PlatformAbstractionX11::ourDisplay, XA_PRIMARY, handle,
        time);
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
        const auto& data = (requestedSelection == XA_PRIMARY) ? gPrimaryText : gClipboardText;
        if (!data.empty()) {
            return AString::fromUtf8(data);
        }
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

void PlatformAbstractionX11::xClipboardClear(Atom selection) {
    if (selection == ourAtoms.clipboard) {
        gClipboardText.clear();
    } else if (selection == XA_PRIMARY) {
        gPrimaryText.clear();
    } else {
        gClipboardText.clear();
        gPrimaryText.clear();
    }
}

static bool convertSingleTarget(Display* dpy, Window requestor, Atom property, Atom target, const std::string& text, Time timestamp) {
    if (property == None) {
        return false;
    }
    Atom utf8String = XInternAtom(dpy, "UTF8_STRING", False);
    Atom textPlain = XInternAtom(dpy, "text/plain", False);
    Atom textPlainUtf8 = XInternAtom(dpy, "text/plain;charset=utf-8", False);
    Atom stringAtom = XInternAtom(dpy, "STRING", False);
    Atom textAtom = XInternAtom(dpy, "TEXT", False);
    Atom targetsAtom = XInternAtom(dpy, "TARGETS", False);
    Atom timestampAtom = XInternAtom(dpy, "TIMESTAMP", False);

    if (target == utf8String || target == textPlain || target == textPlainUtf8 || target == textAtom) {
        Atom responseType = (target == textAtom) ? utf8String : target;
        XChangeProperty(dpy, requestor, property, responseType, 8, PropModeReplace,
                        reinterpret_cast<const unsigned char*>(text.data()), static_cast<int>(text.size()));
        return true;
    }
    if (target == stringAtom || target == XA_STRING) {
        std::string latin1;
        for (char32_t c : AString::fromUtf8(text).toUtf32()) {
            latin1 += (c <= 0xff) ? static_cast<char>(c) : '?';
        }
        XChangeProperty(dpy, requestor, property, target, 8, PropModeReplace,
                        reinterpret_cast<const unsigned char*>(latin1.data()), static_cast<int>(latin1.size()));
        return true;
    }
    if (target == targetsAtom) {
        Atom atoms[] = {
            targetsAtom,
            XInternAtom(dpy, "MULTIPLE", False),
            timestampAtom,
            utf8String,
            textPlain,
            textPlainUtf8,
            textAtom,
            stringAtom,
            XA_STRING,
        };
        XChangeProperty(dpy, requestor, property, XA_ATOM, 32, PropModeReplace,
                        reinterpret_cast<const unsigned char*>(atoms), static_cast<int>(std::size(atoms)));
        return true;
    }
    if (target == timestampAtom) {
        XChangeProperty(dpy, requestor, property, XA_INTEGER, 32, PropModeReplace,
                        reinterpret_cast<const unsigned char*>(&timestamp), 1);
        return true;
    }
    return false;
}

void PlatformAbstractionX11::xHandleClipboard(const XEvent& ev) {
    Atom target = ev.xselectionrequest.target;
    Atom property = ev.xselectionrequest.property;
    Atom multipleAtom = XInternAtom(ourDisplay, "MULTIPLE", False);

    if (property == None) {
        if (target == multipleAtom) {
            property = None;
        } else {
            property = target;
        }
    }

    XSelectionEvent ssev = { 0 };
    ssev.type = SelectionNotify;
    ssev.display = ourDisplay;
    ssev.requestor = ev.xselectionrequest.requestor;
    ssev.selection = ev.xselectionrequest.selection;
    ssev.target = target;
    ssev.property = property;
    ssev.time = ev.xselectionrequest.time;

    if (property == None) {
        ssev.property = None;
        XSendEvent(ourDisplay, ev.xselectionrequest.requestor, False, 0, (XEvent*)&ssev);
        XFlush(ourDisplay);
        return;
    }

    const auto& textToServe = (ev.xselectionrequest.selection == XA_PRIMARY) ? gPrimaryText : gClipboardText;
    Time timestampToServe = (ev.xselectionrequest.selection == XA_PRIMARY) ? gPrimaryTimestamp : gClipboardTimestamp;

    if (target == multipleAtom) {
        Atom actualType;
        int actualFormat;
        unsigned long itemCount = 0, bytesAfter = 0;
        unsigned char* data = nullptr;

        XGetWindowProperty(ourDisplay, ev.xselectionrequest.requestor, property, 0, 0x1000, False,
                           AnyPropertyType, &actualType, &actualFormat, &itemCount, &bytesAfter, &data);

        if (data != nullptr && actualFormat == 32 && itemCount % 2 == 0) {
            auto* atomPairs = reinterpret_cast<Atom*>(data);
            for (unsigned long i = 0; i < itemCount; i += 2) {
                Atom pairTarget = atomPairs[i];
                Atom pairProp = atomPairs[i + 1];
                if (!convertSingleTarget(ourDisplay, ev.xselectionrequest.requestor, pairProp, pairTarget, textToServe, timestampToServe)) {
                    atomPairs[i] = None;
                }
            }
            XChangeProperty(ourDisplay, ev.xselectionrequest.requestor, property, actualType, 32, PropModeReplace,
                            data, static_cast<int>(itemCount));
            XFree(data);
        } else {
            if (data) XFree(data);
            ssev.property = None;
        }
    } else {
        if (!convertSingleTarget(ourDisplay, ev.xselectionrequest.requestor, property, target, textToServe, timestampToServe)) {
            ssev.property = None;
        }
    }

    XSendEvent(ourDisplay, ev.xselectionrequest.requestor, False, 0, (XEvent*)&ssev);
    XFlush(ourDisplay);
}
