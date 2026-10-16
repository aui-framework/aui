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
#include "AUI/Platform/ARenderingContextOptions.h"
#include "OpenGLRenderingContextGtk.h"
#include "AUIWidget.h"
#include "GtkX11Interop.h"
#include <AUI/Util/ARandom.h>
#include <AUI/AppInfo.h>
//#include <adwaita.h>

using namespace aui::gtk4_fake;

#if defined(__clang__)
#pragma clang diagnostic push
#endif
#if defined(__CLION_IDE__) || defined(__CLION_IDE_)
#pragma ide diagnostic ignored "LocalValueEscapesScope"
#endif
bool PlatformAbstractionGtk::windowManagerInitPopup(const IRenderingContext::Init& init) {
    if (!(init.ws & WindowStyle::SYS) || init.parent == nullptr || aui::gtk4_x11::isActive()) {
        // X11 can position and decorate-less toplevels itself.
        return false;
    }
    auto parentContext = dynamic_cast<RenderingContextGtk*>(init.parent->getRenderingContext().get());
    if (!parentContext || !parentContext->auiWidget()) {
        return false;
    }

    static const auto cssOnce = [] {
        // popover chrome (padding, background, shadow) is meant for standard menus; AUI draws everything itself.
        auto provider = gtk_css_provider_new();
        gtk_css_provider_load_from_string(
            provider,
            "popover.aui-popup, popover.aui-popup > contents {"
            "  padding: 0; margin: 0; border: none; border-radius: 0; background: none; box-shadow: none;"
            "}");
        gtk_style_context_add_provider_for_display(
            reinterpret_cast<GdkDisplay*>(gdk_display_get_default()), reinterpret_cast<GtkStyleProvider*>(provider),
            800 /* GTK_STYLE_PROVIDER_PRIORITY_APPLICATION */);
        return true;
    }();
    (void) cssOnce;

    auto popover = gtk_popover_new();
    // the window owns a reference so the popover outlives the parent widget disposal.
    g_object_ref(popover);
    g_object_set_data(G_OBJECT(popover), POPUP_KEY, reinterpret_cast<gpointer>(1));
    gtk_widget_add_css_class(popover, "aui-popup");
    auto popoverHandle = reinterpret_cast<GtkPopover*>(popover);
    gtk_popover_set_has_arrow(popoverHandle, false);
    gtk_popover_set_autohide(popoverHandle, false);
    gtk_popover_set_position(popoverHandle, GTK_POS_BOTTOM);
    nativeHandle(init.window) = reinterpret_cast<GtkWindow*>(popover);
    gtk_popover_set_child(popoverHandle, windowManagerInitGtkBox(init));
    gtk_widget_set_parent(popover, parentContext->auiWidget());
    // the content is laid out right after the creation, i.e., before the popup gets realized; the scaling has to be
    // known by that time, otherwise the popup is sized as if the scale was 100%.
    init.window.updateDpi();
    return true;
}

void PlatformAbstractionGtk::windowManagerInitNativeWindow(const IRenderingContext::Init& init) {
    if (windowManagerInitPopup(init)) {
        return;
    }
    auto window = GTK_WINDOW(gtk_application_window_new(GTK_APPLICATION(*mApplication)));
    if (!!(init.ws & WindowStyle::NO_TITLEBAR)) {
        // a custom titlebar enables client-side decorations; hiding it leaves the window frame (shadow, resize borders)
        // only. This must be done before the window is realized.
        auto titlebar = gtk_header_bar_new();
        gtk_window_set_titlebar(window, titlebar);
        gtkSetTitlebar(window, titlebar);
    }
    windowManagerInitCommon(init, window);
    gtk_window_set_child(window, windowManagerInitGtkBox(init));
    
    // Set WM_CLASS hint after the rendering context is initialized
    if (auto gtkContext = dynamic_cast<RenderingContextGtk*>(init.window.getRenderingContext().get())) {
        gtkSetWindowClassHint(gtkContext, window);
    }
}

void PlatformAbstractionGtk::windowManagerInitCommon(const IRenderingContext::Init& init, GtkWindow* window) {
    nativeHandle(init.window) = window;
    gtk_window_set_default_size(window, init.width, init.height);
    gtk_window_set_title(window, init.name.toStdString().c_str());
    windowSetStyle(init.window, init.ws);

    g_signal_connect(
        window, "close-request", G_CALLBACK(+[](GtkWindow* self, AWindow* window) -> gboolean {
            window->onCloseButtonClicked();
            return true;
        }),
        &init.window);
}

void PlatformAbstractionGtk::gtkSetWindowClassHint(RenderingContextGtk* context, GtkWindow* gtkWindow) {
    // Use application app_id as the window class identifier
    // This is used for identifying the window in window managers and tools on both X11 and Wayland
    context->mWindowClass = aui::app_info::app_id;
    
    // Note: On X11, this would be set as WM_CLASS if using direct X11 window management.
    // On Wayland, GTK automatically uses the app-id from .desktop file or from g_application_id_from_appid.
    // GTK4 handles platform differences automatically.
}

GtkWidget* PlatformAbstractionGtk::windowManagerInitGtkBox(const IRenderingContext::Init& init) const {
    auto box = gtk_box_new(GTK_ORIENTATION_VERTICAL, false);
    gtk_widget_set_margin_start(box, 0);
    gtk_widget_set_margin_end(box, 0);
    gtk_widget_set_margin_top(box, 0);
    gtk_widget_set_margin_bottom(box, 0);
    gtk_box_set_spacing(GTK_BOX(box), 6);
    for (const auto& graphicsApi : ARenderingContextOptions::get().initializationOrder) {
        try {
            auto context = std::visit(
                aui::lambda_overloaded {
                  [](const ARenderingContextOptions::DirectX11&) -> RenderingContextGtk* {
                      throw AException("DirectX is not supported on linux");
                      return nullptr;
                  },
                  [&](const ARenderingContextOptions::OpenGL& config) -> RenderingContextGtk* {
                      auto context = std::make_unique<OpenGLRenderingContextGtk>(config, init.window);
                      auto contextRef = context.get();
                      init.setRenderingContext(std::move(context));
                      return contextRef;
                  },
                  [&](const ARenderingContextOptions::Software&) -> RenderingContextGtk* {
                      // linux provides decent software gles impl
                      auto context = std::make_unique<OpenGLRenderingContextGtk>(ARenderingContextOptions::OpenGL {
                        .majorVersion = 2,
                        .minorVersion = 0,
                      }, init.window);
                      auto contextRef = context.get();
                      init.setRenderingContext(std::move(context));
                      return contextRef;
                  },
                },
                graphicsApi);
            auto auiWidget = aui_widget_new(*context);
            gtk_widget_set_hexpand(auiWidget, true);
            gtk_widget_set_vexpand(auiWidget, true);
            gtk_box_append(GTK_BOX(box), auiWidget);
            return box;
        } catch (const AException& e) {
            ALogger::warn("AWindowManager") << "Unable to initialize graphics API:" << e;
        }
    }
    return box;
}
#if defined(__clang__)
#pragma clang diagnostic pop
#endif

void PlatformAbstractionGtk::windowManagerNotifyProcessMessages() {
    g_main_context_wakeup(mMainContext);
}

void PlatformAbstractionGtk::windowManagerIteration() {
    auto& wm = AWindow::getWindowManager();
    if (!AThread::current()->messageQueueEmpty()) {
        // this is so bad
        if (auto ctx =
            dynamic_cast<RenderingContextGtk*>(wm.getWindows().first()->getRenderingContext().get())) {
            ctx->gtkDoUnderContext([] {
              AThread::processMessages();
            });
        } else {
            AThread::processMessages();
        }
    }
    g_main_context_iteration(mMainContext, true);
}

void PlatformAbstractionGtk::windowManagerLoop() {
    auto& wm = AWindow::getWindowManager();
    wm.start();
    while (wm.shouldKeepRunning()) {
        windowManagerIteration();
    }
}
