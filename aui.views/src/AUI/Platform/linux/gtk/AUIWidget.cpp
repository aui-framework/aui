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

#include "AUIWidget.h"
#include <AUI/GL/gl.h>
#include <cassert>
#include <numeric>
#include <optional>
#include <AUI/Platform/linux/gtk/RenderingContextGtk.h>


struct _AUIWidget {
    GtkWidget parent;
    RenderingContextGtk* renderingContext = nullptr;


    void render() {


    }
};

G_DEFINE_TYPE(AUIWidget, aui_widget, GTK_TYPE_WIDGET);

static void aui_widget_init (AUIWidget* myWidget)
{


    auto motion_controller = gtk_event_controller_motion_new();
    g_signal_connect(motion_controller, "motion", G_CALLBACK(+[](GtkEventControllerMotion* motion, double x, double y, gpointer user_data) {
                         auto* widget = AUI_WIDGET_WIDGET(user_data);

                     }),
                     myWidget);
    gtk_widget_add_controller(GTK_WIDGET(myWidget), motion_controller);

    /*
   gtk_widget_add_tick_callback(GTK_WIDGET(myWidget),
                                [](GtkWidget* widget, GdkFrameClock* frame_clock, gpointer user_data) -> gboolean {
                                    gtk_widget_queue_draw(widget);
                                    return G_SOURCE_CONTINUE; // Return TRUE to keep the callback active
                                },
                                nullptr, nullptr);
   // */
}

static void aui_widget_class_init (AUIWidgetClass* klass)
{
    GTK_WIDGET_CLASS(klass)->realize = [](GtkWidget *widget) {
      GTK_WIDGET_CLASS(aui_widget_parent_class)->realize(widget);
      AUI_WIDGET_WIDGET(widget)->renderingContext->gtkRealize(widget);
    };
    GTK_WIDGET_CLASS(klass)->unrealize = [](GtkWidget *widget) {
      GTK_WIDGET_CLASS(aui_widget_parent_class)->unrealize(widget);
      AUI_WIDGET_WIDGET(widget)->renderingContext->gtkUnrealize(widget);
    };
    GTK_WIDGET_CLASS(klass)->snapshot = [](GtkWidget *widget, GtkSnapshot *snapshot) {
      GTK_WIDGET_CLASS(aui_widget_parent_class)->snapshot(widget, snapshot);
      AUI_WIDGET_WIDGET(widget)->renderingContext->gtkSnapshot(widget, snapshot);
    };

    G_OBJECT_CLASS (klass)->notify = [](GObject *object, GParamSpec *pspec) {
        auto name = std::string_view(pspec->name);
        if (name == "scale-factor") {
            auto widget = AUI_WIDGET_WIDGET(object);

            widget->renderingContext->gtkDoUnderContext([&] {
                widget->renderingContext->window().updateDpi();
            });
            gtk_widget_queue_draw(GTK_WIDGET(widget));
        }
    };
}

AUIWidget* aui_widget_new(RenderingContextGtk& renderingContext)
{
    auto widget = AUI_WIDGET_WIDGET(g_object_new(AUI_WIDGET_TYPE, nullptr));
    widget->renderingContext = &renderingContext;
    renderingContext.mAUIWidget = widget;
    return widget;
}

void RenderingContextGtk::gtkDoUnderContext(const std::function<void()>& callback) {
    callback();
}
