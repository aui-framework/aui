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

#include "PlatformAbstractionGtk.h"
#include "RenderingContextGtk.h"
#include <adwaita.h>

// hack: we sort of implementing g_application_run here.
// particularly:
// - command line parsing is already done by AUI_ENTRY
// - acquiring GMainContext in constructor
// - registering the application
// - windowManagerLoop handles application loop
// - releasing acquired context in destructor
// See https://github.com/aui-framework/aui/issues/468

PlatformAbstractionGtk::PlatformAbstractionGtk()
  : mApplication([] {
      gtk_init();
      return G_APPLICATION(adw_application_new(nullptr, G_APPLICATION_DEFAULT_FLAGS));
  }()), mMainContext(g_main_context_default()) {
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
    // maybe we might want to delegate command line handling to GApplication
    while (g_main_context_iteration (mMainContext, false));

    // TODO at the moment aui does not really support colors schemes, so we force light mode
    adw_style_manager_set_color_scheme(adw_application_get_style_manager(ADW_APPLICATION(mApplication)),
                                       ADW_COLOR_SCHEME_FORCE_LIGHT);
}

PlatformAbstractionGtk::~PlatformAbstractionGtk() {
    g_signal_emit_by_name(mApplication, "quit");
    while (g_main_context_iteration (mMainContext, false));
    mApplication = nullptr;
    while (g_main_context_iteration (mMainContext, false));
    g_main_context_release(mMainContext);
}

_<ACursor::Custom> PlatformAbstractionGtk::createCustomCursor(AImageView image) { return _<ACursor::Custom>(); }
void PlatformAbstractionGtk::applyNativeCursor(const ACursor &cursor, AWindow *pWindow) {}
void PlatformAbstractionGtk::copyToClipboard(const AString &text) {}
AString PlatformAbstractionGtk::pasteFromClipboard() { return AString(); }
glm::ivec2 PlatformAbstractionGtk::desktopGetMousePosition() { return glm::ivec2(); }
void PlatformAbstractionGtk::desktopSetMousePosition(glm::ivec2 pos) {}
float PlatformAbstractionGtk::platformGetDpiRatio() {
    return 1.f;
}
void PlatformAbstractionGtk::windowSetStyle(AWindow &window, WindowStyle ws) {
    gtk_window_set_resizable(nativeHandle(window), !bool(ws & WindowStyle::NO_RESIZE));
    gtk_window_set_modal(nativeHandle(window), bool(ws & WindowStyle::NO_MINIMIZE_MAXIMIZE));
    if (bool(ws & WindowStyle::MODAL) && parentWindow(window)) {
        gtk_window_set_transient_for(nativeHandle(window), nativeHandle(*parentWindow(window)));
    } else {
        gtk_window_set_transient_for(nativeHandle(window), nullptr);
    }
}
float PlatformAbstractionGtk::windowFetchDpiFromSystem(AWindow &window) { return 0; }
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
    auto surface = gtk_native_get_surface(gtk_widget_get_native(GTK_WIDGET(nativeHandle(window))));
    if (surface == nullptr) {
        return 1.0;
    }
    return gdk_surface_get_scale(surface);
}

