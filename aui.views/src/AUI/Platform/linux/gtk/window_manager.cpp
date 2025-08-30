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
//#include <adwaita.h>

using namespace aui::gtk4_fake;

#if defined(__clang__)
#pragma clang diagnostic push
#endif
#if defined(__CLION_IDE__) || defined(__CLION_IDE_)
#pragma ide diagnostic ignored "LocalValueEscapesScope"
#endif
void PlatformAbstractionGtk::windowManagerInitNativeWindow(const IRenderingContext::Init& init) {
    auto window = GTK_WINDOW(gtk_application_window_new(GTK_APPLICATION(*mApplication)));
    windowManagerInitCommon(init, window);
    gtk_window_set_child(window, windowManagerInitGtkBox(init));
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
    while (wm.isLoopRunning() && !wm.getWindows().empty()) {
        windowManagerIteration();
    }
}
