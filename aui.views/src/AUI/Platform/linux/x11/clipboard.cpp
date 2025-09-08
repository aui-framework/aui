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
}

void PlatformAbstractionX11::copyToClipboard(const AString& text) {
    auto basicWindow = AWindow::current();
    auto auiWindow = dynamic_cast<AWindow*>(basicWindow);
    if (!auiWindow)
        return;
    gClipboardText = text.toStdString();
    XSetSelectionOwner(
        PlatformAbstractionX11::ourDisplay, PlatformAbstractionX11::ourAtoms.clipboard, nativeHandle(*auiWindow),
        CurrentTime);
}

AString PlatformAbstractionX11::pasteFromClipboard() {
    auto owner = XGetSelectionOwner(PlatformAbstractionX11::ourDisplay, PlatformAbstractionX11::ourAtoms.clipboard);
    if (owner == None)
    {
        return {};
    }
    auto basicWindow = AWindow::current();
    auto auiWindow = dynamic_cast<AWindow*>(basicWindow);
    if (!auiWindow)
        return {};
    auto nativeHandle = auiWindow->getNativeHandle();
    AUI_ASSERT(nativeHandle);

    XConvertSelection(PlatformAbstractionX11::ourDisplay, PlatformAbstractionX11::ourAtoms.clipboard, PlatformAbstractionX11::ourAtoms.utf8String, PlatformAbstractionX11::ourAtoms.auiClipboard, nativeHandle,
                      CurrentTime);

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


    char* targetName = XGetAtomName(ourDisplay, ev.xselectionrequest.target);
    char* propertyName = XGetAtomName(ourDisplay, ev.xselectionrequest.property);
    ALogger::info("{}: {}"_format(targetName, propertyName));
    XFree(targetName);
    XFree(propertyName);
    if (ev.xselectionrequest.target == ourAtoms.utf8String ||
        ev.xselectionrequest.target == ourAtoms.textPlain ||
        ev.xselectionrequest.target == ourAtoms.textPlainUtf8) { // check for UTF8_STRING
        XChangeProperty(ourDisplay,
                        ev.xselectionrequest.requestor,
                        ev.xselectionrequest.property,
                        ev.xselectionrequest.target,
                        8,
                        PropModeReplace,
                        (unsigned char*) gClipboardText.c_str(),
                        gClipboardText.size());
    } else if (ev.xselectionrequest.target == ourAtoms.targets) { // data type request
        Atom atoms[] = {
            XInternAtom(ourDisplay, "TIMESTAMP", false),
            XInternAtom(ourDisplay, "TARGETS", false),
            XInternAtom(ourDisplay, "SAVE_TARGETS", false),
            XInternAtom(ourDisplay, "MULTIPLE", false),
            XInternAtom(ourDisplay, "STRING", false),
            XInternAtom(ourDisplay, "UTF8_STRING", false),
            XInternAtom(ourDisplay, "text/plain", false),
            XInternAtom(ourDisplay, "text/plain;charset=utf-8", false),
        };
        XChangeProperty(ourDisplay,
                        ev.xselectionrequest.requestor,
                        ev.xselectionrequest.property,
                        ev.xselectionrequest.target,
                        8,
                        PropModeReplace,
                        (unsigned char*) atoms,
                        sizeof(atoms));
    }

    XSelectionEvent ssev;
    ssev.type = SelectionNotify;
    ssev.requestor = ev.xselectionrequest.requestor;
    ssev.selection = ev.xselectionrequest.selection;
    ssev.target = ev.xselectionrequest.target;
    ssev.property = ev.xselectionrequest.property;
    ssev.time = ev.xselectionrequest.time;

    XSendEvent(ourDisplay, ev.xselectionrequest.requestor, True, NoEventMask, (XEvent *)&ssev);
}
