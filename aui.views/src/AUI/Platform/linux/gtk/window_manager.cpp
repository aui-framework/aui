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
#include <adwaita.h>

static constexpr auto USE_ADWAITA = true;

#pragma clang diagnostic push
#pragma ide diagnostic ignored "LocalValueEscapesScope"
void PlatformAbstractionGtk::windowManagerInitNativeWindow(const IRenderingContext::Init& init) {
    if (auto display = gdk_display_get_default()) {
        // vulkan? no thanks
        g_object_set_data_full (G_OBJECT (display), "gsk-renderer", g_strdup("ngl"), g_free);
    }

    auto window =
        USE_ADWAITA ? GTK_WINDOW(adw_application_window_new(GTK_APPLICATION(*mApplication)))
                    : GTK_WINDOW(gtk_application_window_new(GTK_APPLICATION(*mApplication)));
    nativeHandle(init.window) = window;
    gtk_window_set_default_size(window, init.width, init.height);
    gtk_window_set_title(window, init.name.toStdString().c_str());

    auto box = gtk_box_new(GTK_ORIENTATION_VERTICAL, FALSE);
    gtk_widget_set_margin_start(box, 0);
    gtk_widget_set_margin_end(box, 0);
    gtk_widget_set_margin_top(box, 0);
    gtk_widget_set_margin_bottom(box, 0);
    gtk_box_set_spacing(GTK_BOX(box), 6);

    windowSetStyle(init.window, init.ws);

    if (USE_ADWAITA) {
        auto toolbar = adw_toolbar_view_new();
        auto header = adw_header_bar_new();
        adw_toolbar_view_add_top_bar(ADW_TOOLBAR_VIEW(toolbar), header);
        adw_toolbar_view_set_top_bar_style(ADW_TOOLBAR_VIEW(toolbar), ADW_TOOLBAR_RAISED_BORDER);
        adw_toolbar_view_set_extend_content_to_top_edge(ADW_TOOLBAR_VIEW(toolbar), false);
        adw_toolbar_view_set_content(ADW_TOOLBAR_VIEW(toolbar), box);
        adw_application_window_set_content(ADW_APPLICATION_WINDOW(window), toolbar);
    } else {
        // TODO
    }
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
            auto auiWidget = GTK_WIDGET(aui_widget_new(*context));
            gtk_widget_set_hexpand(auiWidget, true);
            gtk_widget_set_vexpand(auiWidget, true);
            gtk_box_append(GTK_BOX(box), auiWidget);
            return;
        } catch (const AException& e) {
            ALogger::warn("AWindowManager") << "Unable to initialize graphics API:" << e;
        }
    }
}
#pragma clang diagnostic pop

void PlatformAbstractionGtk::windowManagerNotifyProcessMessages() {
    g_main_context_wakeup(mMainContext);
}

void PlatformAbstractionGtk::windowManagerLoop() {
    auto& wm = AWindow::getWindowManager();
    wm.start();
    while (wm.isLoopRunning() && !wm.getWindows().empty()) {
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
}
