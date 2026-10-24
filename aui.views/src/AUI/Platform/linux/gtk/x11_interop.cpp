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

#include <dlfcn.h>
#include <AUI/Image/AImage.h>
#include "GtkX11Interop.h"
#include "gtk_functions.h"

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xrandr.h>

namespace aui::gtk4_fake {
extern void* handle;
}

namespace {

using namespace aui::gtk4_fake;

struct Backend {
    GType (*displayGetType)() = nullptr;
    Display* (*displayGetXDisplay)(GdkDisplay*) = nullptr;
    ::Window (*surfaceGetXid)(GdkSurface*) = nullptr;
    GdkDisplay* display = nullptr;
    Display* xdisplay = nullptr;

    explicit operator bool() const { return xdisplay != nullptr; }
};

Backend backend() {
    // these symbols exist only if GTK was built with X11 backend.
    // the symbols never change, so they are looked up once.
    static const Backend symbols = [] {
        Backend s;
        s.displayGetType = reinterpret_cast<GType (*)()>(dlsym(handle, "gdk_x11_display_get_type"));
        s.displayGetXDisplay =
            reinterpret_cast<Display* (*)(GdkDisplay*)>(dlsym(handle, "gdk_x11_display_get_xdisplay"));
        s.surfaceGetXid = reinterpret_cast<::Window (*)(GdkSurface*)>(dlsym(handle, "gdk_x11_surface_get_xid"));
        return s;
    }();
    Backend b = symbols;
    if (!b.displayGetType || !b.displayGetXDisplay || !b.surfaceGetXid) {
        return b;
    }
    auto display = reinterpret_cast<GdkDisplay*>(gdk_display_get_default());
    if (!display || !g_type_check_instance_is_a(reinterpret_cast<GTypeInstance*>(display), b.displayGetType())) {
        return b;
    }
    b.display = display;
    b.xdisplay = b.displayGetXDisplay(display);
    return b;
}

struct Target {
    Backend b;
    ::Window window = 0;
    explicit operator bool() const { return b && window; }
};

Target target(GdkSurface* surface) {
    Target t { backend() };
    if (!t.b || !surface) {
        return t;
    }
    t.window = t.b.surfaceGetXid(surface);
    return t;
}

glm::ivec2 positionOf(const Target& t) {
    int x = 0, y = 0;
    ::Window child;
    XTranslateCoordinates(t.b.xdisplay, t.window, XDefaultRootWindow(t.b.xdisplay), 0, 0, &x, &y, &child);
    return { x, y };
}

glm::ivec2 sizeOf(const Target& t) {
    XWindowAttributes attrs;
    XGetWindowAttributes(t.b.xdisplay, t.window, &attrs);
    return { attrs.width, attrs.height };
}

}   // namespace

namespace aui::gtk4_x11 {

bool isActive() { return bool(backend()); }

std::optional<glm::ivec2> windowPosition(GdkSurface* surface) {
    auto t = target(surface);
    if (!t) {
        return std::nullopt;
    }
    return positionOf(t);
}

bool windowMoveResize(GdkSurface* surface, int x, int y, int width, int height, bool overrideRedirect) {
    auto t = target(surface);
    if (!t) {
        return false;
    }
    if (overrideRedirect) {
        // popup menus must neither be decorated nor steal the focus. Only possible for not yet mapped windows.
        XSetWindowAttributes attrs;
        attrs.override_redirect = True;
        XChangeWindowAttributes(t.b.xdisplay, t.window, CWOverrideRedirect, &attrs);
    }
    XWindowChanges changes;
    changes.x = x;
    changes.y = y;
    // XConfigureWindow rejects zero sizes with BadValue
    changes.width = std::max(width, 1);
    changes.height = std::max(height, 1);
    changes.stack_mode = Above;
    XConfigureWindow(t.b.xdisplay, t.window, CWX | CWY | CWWidth | CWHeight | CWStackMode, &changes);
    XSync(t.b.xdisplay, False);
    return true;
}

bool windowMoveToCenter(GdkSurface* surface, GdkSurface* parentSurface) {
    auto t = target(surface);
    if (!t) {
        return false;
    }
    glm::ivec2 areaPosition { 0, 0 };
    glm::ivec2 areaSize { DisplayWidth(t.b.xdisplay, 0), DisplayHeight(t.b.xdisplay, 0) };

    if (auto parent = target(parentSurface)) {
        areaPosition = positionOf(parent);
        areaSize = sizeOf(parent);
    } else {
        // center on the primary monitor
        int count = 0;
        if (auto monitors = XRRGetMonitors(t.b.xdisplay, XDefaultRootWindow(t.b.xdisplay), True, &count)) {
            for (int i = 0; i < count; ++i) {
                if (i == 0 || monitors[i].primary) {
                    areaPosition = { monitors[i].x, monitors[i].y };
                    areaSize = { monitors[i].width, monitors[i].height };
                }
            }
            XRRFreeMonitors(monitors);
        }
    }

    auto position = areaPosition + (areaSize - sizeOf(t)) / 2;
    XMoveWindow(t.b.xdisplay, t.window, position.x, position.y);
    XFlush(t.b.xdisplay);
    return true;
}

bool windowSetIcon(GdkSurface* surface, const AImage& image) {
    auto t = target(surface);
    if (!t) {
        return false;
    }
    std::vector<unsigned long> data(image.width() * image.height() + 2);
    data[0] = image.width();
    data[1] = image.height();
    size_t idx = 2;
    for (size_t y = 0; y < image.height(); y++) {
        for (size_t x = 0; x < image.width(); x++) {
            AColor col = image.get(glm::uvec2(x, y));
            data[idx++] = static_cast<unsigned long>(uint8_t(col.a * 255.0f)) << 24 |
                          static_cast<unsigned long>(uint8_t(col.r * 255.0f)) << 16 |
                          static_cast<unsigned long>(uint8_t(col.g * 255.0f)) << 8 |
                          static_cast<unsigned long>(uint8_t(col.b * 255.0f));
        }
    }
    XChangeProperty(
        t.b.xdisplay, t.window, XInternAtom(t.b.xdisplay, "_NET_WM_ICON", False), XA_CARDINAL, 32, PropModeReplace,
        reinterpret_cast<unsigned char*>(data.data()), static_cast<int>(data.size()));
    XFlush(t.b.xdisplay);
    return true;
}

bool windowSetMaxSize(GdkSurface* surface, glm::ivec2 maxSize) {
    auto t = target(surface);
    if (!t) {
        return false;
    }
    XSizeHints* hints = XAllocSizeHints();
    long supplied;
    XGetWMNormalHints(t.b.xdisplay, t.window, hints, &supplied);
    hints->flags |= PMaxSize;
    hints->max_width = maxSize.x == -1 ? 100000 : maxSize.x;
    hints->max_height = maxSize.y == -1 ? 100000 : maxSize.y;
    XSetWMNormalHints(t.b.xdisplay, t.window, hints);
    XFree(hints);
    XFlush(t.b.xdisplay);
    return true;
}

std::optional<glm::ivec2> pointerPosition() {
    auto b = backend();
    if (!b) {
        return std::nullopt;
    }
    glm::ivec2 p;
    ::Window w;
    int unused1;
    unsigned unused2;
    XQueryPointer(b.xdisplay, XDefaultRootWindow(b.xdisplay), &w, &w, &p.x, &p.y, &unused1, &unused1, &unused2);
    return p;
}

bool warpPointer(glm::ivec2 position) {
    auto b = backend();
    if (!b) {
        return false;
    }
    XWarpPointer(b.xdisplay, None, XDefaultRootWindow(b.xdisplay), 0, 0, 0, 0, position.x, position.y);
    XFlush(b.xdisplay);
    return true;
}

}   // namespace aui::gtk4_x11
