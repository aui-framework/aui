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

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xresource.h>
#include "PlatformAbstractionX11.h"

glm::ivec2 PlatformAbstractionX11::desktopGetMousePosition() {
    glm::ivec2 p;
    Window w;
    int unused1;
    unsigned unused2;
    XQueryPointer(
        PlatformAbstractionX11::ourDisplay, XRootWindow(PlatformAbstractionX11::ourDisplay, 0), &w, &w, &p.x, &p.y,
        &unused1, &unused1, &unused2);
    return p;
}

void PlatformAbstractionX11::desktopSetMousePosition(glm::ivec2 pos) {
    auto rootWindow = XRootWindow(PlatformAbstractionX11::ourDisplay, 0);
    XSelectInput(PlatformAbstractionX11::ourDisplay, rootWindow, KeyReleaseMask);
    XWarpPointer(PlatformAbstractionX11::ourDisplay, None, rootWindow, 0, 0, 0, 0, pos.x, pos.y);
    XFlush(PlatformAbstractionX11::ourDisplay);
}

float PlatformAbstractionX11::windowFetchDpiFromSystem(AWindow& window) {
    ensureXLibInitialized();
    if (ourDisplay == nullptr) return 1.f;

    static auto value = [] {
      char* resourceString = XResourceManagerString(PlatformAbstractionX11::ourDisplay);

      if (!resourceString) {
          return 1.f;
      }
      XrmInitialize();

      XrmValue value;
      char* type = nullptr;

      auto db = aui::ptr::manage_unique(XrmGetStringDatabase(resourceString), XrmDestroyDatabase);

      if (XrmGetResource(db.get(), "Xft.dpi", "String", &type, &value)) {
          if (value.addr) {
              return float(atof(value.addr)) / 96.f;
          }
      }
      return 1.f;
    }();

    return value;
}