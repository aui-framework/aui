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
    if (!init.popup || init.parent == nullptr || aui::gtk4_x11::isActive()) {
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
    if (!init.popup) {
        // a custom titlebar enables client-side decorations; it also lets us control the window buttons and hide the
        // titlebar leaving the window frame (shadow, resize borders) only. This must be done before the window is
        // realized.
        auto titlebar = gtk_header_bar_new();
        gtk_window_set_titlebar(window, titlebar);
        gtkSetTitlebar(window, titlebar);
    }
    windowManagerInitCommon(init, window);
    gtk_window_set_child(window, windowManagerInitContent(init, window));

    // Set WM_CLASS hint after the rendering context is initialized
    if (auto gtkContext = dynamic_cast<RenderingContextGtk*>(init.window.getRenderingContext().get())) {
        gtkSetWindowClassHint(gtkContext, window);
    }
}

AString PlatformAbstractionGtk::gtkFilteredLayout(const AWindow::Init::Buttons& buttons) {
    // the layout is system-wide ("close:minimize,maximize", "appmenu:close", ...); drop the disabled buttons from it.
    char* systemLayout = nullptr;
    if (auto settings = gtk_settings_get_default()) {
        g_object_get(settings, "gtk-decoration-layout", &systemLayout, nullptr);
    }
    AString layout = systemLayout ? AString(systemLayout) : AString("appmenu:minimize,maximize,close");
    g_free(systemLayout);

    AString result;
    AString token;
    const auto flush = [&] {
        if (token.empty()) {
            return;
        }
        const bool enabled = (token != "minimize" || buttons.minimize) && (token != "maximize" || buttons.maximize) &&
                             (token != "close" || buttons.close);
        if (enabled) {
            if (!result.empty() && result.back() != ':') {
                result += ',';
            }
            result += token;
        }
        token.clear();
    };
    for (auto c : layout) {
        if (c == ':' || c == ',') {
            flush();
            if (c == ':') {
                result += ':';
            }
        } else {
            token += c;
        }
    }
    flush();
    return result;
}

void PlatformAbstractionGtk::gtkApplyButtons(aui::gtk4_fake::GtkWidget* titlebar, const AWindow::Init::Buttons& buttons) {
    const auto utf8 = gtkFilteredLayout(buttons).toStdString();
    g_object_set(titlebar, "decoration-layout", utf8.c_str(), nullptr);
}

void PlatformAbstractionGtk::windowManagerInitCommon(const IRenderingContext::Init& init, GtkWindow* window) {
    nativeHandle(init.window) = window;
    gtk_window_set_default_size(window, init.width, init.height);
    gtk_window_set_title(window, init.name.toStdString().c_str());

    const auto& windowInit = init.window.init();
    gtk_window_set_resizable(window, windowInit.resizable);

    // GTK4 has no way to hide only minimize/maximize buttons of a non-resizable window besides the layout; AWindow's
    // Buttons are applied via decoration-layout of the titlebar.
    bool decorated = true;
    bool titlebarVisible = true;
    std::visit(
        aui::lambda_overloaded {
          [&](const AWindow::Init::DecorationDefault& d) {
              if (auto titlebar = gtkTitlebar(window)) {
                  gtkApplyButtons(titlebar, d.buttons);
              }
          },
          [&](const AWindow::Init::DecorationExtended&) {
              // AUI draws its own titlebar contents (ATitlebar).
              titlebarVisible = false;
          },
          [&](const AWindow::Init::DecorationBorderless& d) {
              // CSD without a titlebar leaves us with the shadow, rounded corners and resize borders only.
              titlebarVisible = false;
              decorated = d.shadow;
          },
        },
        windowInit.decoration);
    if (init.popup) {
        decorated = false;
        titlebarVisible = false;
        gtk_window_set_deletable(window, false);
    }
    if (auto titlebar = gtkTitlebar(window)) {
        // adw windows ignore gtk_window_set_decorated, so the titlebar has to be hidden explicitly.
        gtk_widget_set_visible(titlebar, titlebarVisible);
    } else if (!titlebarVisible) {
        decorated = false;
    }
    gtk_window_set_decorated(window, decorated);

    if (windowInit.parent) {
        gtk_window_set_transient_for(window, nativeHandle(windowInit.parent->window));
        gtk_window_set_modal(window, windowInit.parent->modal);
    }

    g_signal_connect(
        window, "close-request", G_CALLBACK(+[](GtkWindow* self, AWindow* window) -> gboolean {
            // we destroy the window ourselves (AWindow::close)
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
            gtk_widget_add_css_class(auiWidget, "aui-surface");
            gtk_widget_set_overflow(auiWidget, GTK_OVERFLOW_HIDDEN);
            gtk_widget_add_css_class(box, "aui-surface-host");
            gtk_widget_set_overflow(box, GTK_OVERFLOW_HIDDEN);
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

GtkWidget* PlatformAbstractionGtk::windowManagerInitContent(const IRenderingContext::Init& init, GtkWindow* window) const {
    static const auto cssOnce = [] {
        // The AUI surface is clipped by the rounded corners of the client-side decorated window. The radius is taken
        // from the theme (Adwaita/adw), and becomes 0 on its own when the window is maximized or tiled.
        auto provider = gtk_css_provider_new();
        gtk_css_provider_load_from_string(
            provider,
            ".aui-round, .aui-round > .aui-surface-host, .aui-round .aui-surface {"
            "  border-radius: inherit;"
            "}"
            ".aui-round-bottom, .aui-round-bottom > .aui-surface-host, .aui-round-bottom .aui-surface {"
            "  border-bottom-left-radius: inherit; border-bottom-right-radius: inherit;"
            "}");
        gtk_style_context_add_provider_for_display(
            reinterpret_cast<GdkDisplay*>(gdk_display_get_default()), reinterpret_cast<GtkStyleProvider*>(provider),
            800 /* GTK_STYLE_PROVIDER_PRIORITY_APPLICATION */);
        return true;
    }();
    (void) cssOnce;

    auto box = windowManagerInitGtkBox(init);
    if (init.popup) {
        return box;
    }
    const auto& decoration = init.window.init().decoration;
    const bool extended = std::holds_alternative<AWindow::Init::DecorationExtended>(decoration);
    // with the default decoration, the header bar is on the top; only the bottom corners are round.
    const bool bottomOnly = std::holds_alternative<AWindow::Init::DecorationDefault>(decoration);
    const char* roundClass = bottomOnly ? "aui-round-bottom" : "aui-round";

    GtkWidget* result = box;
    if (const auto* e = std::get_if<AWindow::Init::DecorationExtended>(&decoration)) {
        // content extends into the titlebar; real GtkWindowControls are put over it, so the order and the side of
        // the buttons match the desktop environment, and the hover/press states are native.
        auto overlay = gtk_overlay_new();
        gtk_widget_set_hexpand(overlay, true);
        gtk_widget_set_vexpand(overlay, true);
        gtk_overlay_set_child(overlay, box);
        gtk_widget_set_overflow(overlay, GTK_OVERFLOW_HIDDEN);
        const auto layout = gtkFilteredLayout(e->buttons).toStdString();
        const auto add = [&](GtkPackType side, GtkAlign align, const char* key) {
            auto controls = gtk_window_controls_new(side);
            g_object_set(controls, "decoration-layout", layout.c_str(), nullptr);
            gtk_widget_set_halign(controls, align);
            gtk_widget_set_valign(controls, GTK_ALIGN_START);
            gtk_overlay_add_overlay(overlay, controls);
            g_object_set_data(G_OBJECT(window), key, controls);
        };
        add(GTK_PACK_START, GTK_ALIGN_START, CONTROLS_START_KEY);
        add(GTK_PACK_END, GTK_ALIGN_END, CONTROLS_END_KEY);
        result = overlay;
    }
    (void) extended;
    gtk_widget_add_css_class(result, roundClass);
    return result;
}
