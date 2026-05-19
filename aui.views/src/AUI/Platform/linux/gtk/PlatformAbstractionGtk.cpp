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
#include "PlatformAbstractionGtk.h"
#include "RenderingContextGtk.h"
#include "gtk_functions.h"
#include <AUI/AppInfo.h>
#include <AUI/Platform/AWindow.h>

// hack: we sort of implementing g_application_run here.
// particularly:
// - command line parsing is already done by AUI_ENTRY
// - acquiring GMainContext in constructor
// - registering the application
// - windowManagerLoop handles application loop
// - releasing acquired context in destructor
// See https://github.com/aui-framework/aui/issues/468

namespace aui::gtk4_fake {
extern void* handle;
}

using namespace aui::gtk4_fake;

PlatformAbstractionGtk::PlatformAbstractionGtk() : mMainContext(g_main_context_default()) {
    handle = dlopen("libgtk-4.so.1", RTLD_LAZY | RTLD_GLOBAL);
    if (!handle) {
        throw AException("failed to load libgtk-4.so.1: {}"_format(dlerror()));
    }
    gtk_init();

    if (auto display = gdk_display_get_default()) {
        // vulkan? no thanks
        g_object_set_data_full(G_OBJECT(display), "gsk-renderer", g_strdup("ngl"), g_free);
    }
}

void PlatformAbstractionGtk::init() {
    if (!mApplication) {
        // Use app_id from AppInfo as the application identifier for GTK
        // This will be used as the WM_CLASS on X11 and app-id on Wayland
        mApplication = G_APPLICATION(gtk_application_new(
            aui::app_info::app_id.toStdString().c_str(),
            static_cast<GApplicationFlags>(0)));
    }
    g_signal_connect(
        mApplication, "activate",
        [] {
          /* just a stub to silence warning */
        },
        nullptr);

    if (!g_main_context_acquire(mMainContext)) {
        throw AException("failed to acquire GMainContext");
    }

    if (GError* error = nullptr; !g_application_register(mApplication, nullptr, &error)) {
        AUI_DEFER { g_error_free(error); };
        throw AException("failed to register GApplication: {}"_format(error->message));
    }
    
    // Monitor gtk-xft-dpi changes
    setupDpiChangeMonitoring();
    
    // maybe we might want to delegate command line handling to GApplication
    while (g_main_context_iteration (mMainContext, false));
}

PlatformAbstractionGtk::~PlatformAbstractionGtk() {
    while (g_main_context_iteration (mMainContext, false));
    mApplication = nullptr;
    while (g_main_context_iteration (mMainContext, false));
    g_main_context_release(mMainContext);
}

_<ACursor::Custom> PlatformAbstractionGtk::createCustomCursor(AImageView image) { return _<ACursor::Custom>(); }
void PlatformAbstractionGtk::applyNativeCursor(const ACursor &cursor, AWindow *pWindow) {}
void PlatformAbstractionGtk::setClipboardText(const AString &text) {}
AString PlatformAbstractionGtk::getClipboardText() { return AString(); }
glm::ivec2 PlatformAbstractionGtk::desktopGetMousePosition() { return glm::ivec2(); }
void PlatformAbstractionGtk::desktopSetMousePosition(glm::ivec2 pos) {}
void PlatformAbstractionGtk::windowSetStyle(AWindow &window, WindowStyle ws) {
    gtk_window_set_resizable(nativeHandle(window), !bool(ws & WindowStyle::NO_RESIZE));
    gtk_window_set_modal(nativeHandle(window), bool(ws & WindowStyle::NO_MINIMIZE_MAXIMIZE));
    if (bool(ws & WindowStyle::MODAL) && parentWindow(window)) {
        gtk_window_set_transient_for(nativeHandle(window), nativeHandle(*parentWindow(window)));
    } else {
        gtk_window_set_transient_for(nativeHandle(window), nullptr);
    }
}
float PlatformAbstractionGtk::windowFetchDpiFromSystem(AWindow &window) {
    auto surface = gtk_native_get_surface(gtk_widget_get_native(GTK_WIDGET(nativeHandle(window))));
    if (surface == nullptr) {
        return 1.0f;
    }
    return float(gdk_surface_get_scale(surface));
}
void PlatformAbstractionGtk::windowRestore(AWindow &window) {}
void PlatformAbstractionGtk::windowMinimize(AWindow &window) {}
bool PlatformAbstractionGtk::windowIsMinimized(AWindow &window) const { return false; }
bool PlatformAbstractionGtk::windowIsMaximized(AWindow &window) const { return false; }
void PlatformAbstractionGtk::windowMaximize(AWindow &window) {}
glm::ivec2 PlatformAbstractionGtk::windowGetPosition(AWindow &window) const { return glm::ivec2(); }
void PlatformAbstractionGtk::windowFlagRedraw(AWindow &window) {
    window.getThread()->enqueue([&window] {
      if (auto ctx = dynamic_cast<RenderingContextGtk*>(window.getRenderingContext().get())) {
          gtk_widget_queue_draw(GTK_WIDGET(ctx->auiWidget()));
      }
    });
}

void PlatformAbstractionGtk::windowShow(AWindow &window) {
    gtk_window_present(nativeHandle(window));
}

void PlatformAbstractionGtk::windowSetSize(AWindow &window, glm::ivec2 size) {}
void PlatformAbstractionGtk::windowSetGeometry(AWindow &window, int x, int y, int width, int height) {}
void PlatformAbstractionGtk::windowSetIcon(AWindow &window, const AImage &image) {}
void PlatformAbstractionGtk::windowHide(AWindow &window) {}
void PlatformAbstractionGtk::windowBlockUserInput(AWindow &window, bool blockUserInput) {}
void PlatformAbstractionGtk::windowAllowDragNDrop(AWindow &window) {}
void PlatformAbstractionGtk::windowShowTouchscreenKeyboardImpl(AWindow &window) {}
void PlatformAbstractionGtk::windowHideTouchscreenKeyboardImpl(AWindow &window) {}
void PlatformAbstractionGtk::windowMoveToCenter(AWindow &window) {}
void PlatformAbstractionGtk::windowQuit(AWindow &window) {
    gtk_window_close(nativeHandle(window));
}
void PlatformAbstractionGtk::windowAnnounceMinMaxSize(AWindow &window) {}

float PlatformAbstractionGtk::windowGetDpiRatio(AWindow &window) {
    float scale = 1.0f;
    
    // Try to get surface scale first
    auto surface = gtk_native_get_surface(gtk_widget_get_native(GTK_WIDGET(nativeHandle(window))));
    if (surface != nullptr) {
        scale = gdk_surface_get_scale(surface);
    }
    
    // Also apply GTK text scale settings
    auto settings = gtk_settings_get_default();
    if (settings != nullptr) {
        gint xft_dpi = 0;
        g_object_get(settings, "gtk-xft-dpi", &xft_dpi, nullptr);
        if (xft_dpi > 0) {
            // gtk-xft-dpi is in units of 1/1024 of an inch, normalize to 96 dpi base
            float text_scale = xft_dpi / 1024.0f / 96.0f;
            scale *= text_scale;
        }
    }
    
    return scale;
}

void PlatformAbstractionGtk::setupDpiChangeMonitoring() {
    auto settings = gtk_settings_get_default();
    if (settings == nullptr) {
        return;
    }
    
    // Listen to changes in gtk-xft-dpi setting
    g_signal_connect(
        settings, "notify::gtk-xft-dpi",
        G_CALLBACK(+[](GObject* object, GParamSpec* pspec, gpointer user_data) {
            // Trigger updateDpi() on current window
            if (auto currentWindow = dynamic_cast<AWindow*>(AWindow::current())) {
                currentWindow->getThread()->enqueue([currentWindow]() {
                    currentWindow->updateDpi();
                });
            }
        }),
        nullptr);
}
