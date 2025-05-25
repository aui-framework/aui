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
#include "gtk_functions.h"
#include <AUI/GL/gl.h>
#include <cassert>
#include <numeric>
#include <optional>
#include <AUI/Platform/linux/gtk/RenderingContextGtk.h>
#include <AUI/Platform/linux/gtk/PlatformAbstractionGtk.h>

using namespace aui::gtk4_fake;

#define AUI_WIDGET_TYPE (aui_widget_get_type ())
G_DECLARE_FINAL_TYPE (AUIWidget, aui_widget, AUI_WIDGET, WIDGET, GtkWidget)

extern bool gKeyStates[AInput::KEYCOUNT + 1];

struct _AUIWidget {
    GtkWidget parent;
    RenderingContextGtk* renderingContext = nullptr;
    GtkIMContext* imContext = nullptr;

    ~_AUIWidget() { g_clear_object(&imContext); }
};

G_DEFINE_TYPE(AUIWidget, aui_widget, GTK_TYPE_WIDGET);

static GtkWidget* getEventWidget(GdkEvent* event) {
    auto surface = gdk_event_get_surface(event);
    if (surface && !gdk_surface_is_destroyed(surface))
        return GTK_WIDGET(gtk_native_get_for_surface(surface));

    return nullptr;
}

static glm::vec2 getEventPosition(GdkEvent* event, GtkWidget* widget) {
    double eventX, eventY;
    if (!gdk_event_get_position(event, &eventX, &eventY))
        return {};

    auto eventWidget = getEventWidget(event);
    auto native = gtk_widget_get_native(eventWidget);
    double nx, ny;
    gtk_native_get_surface_transform(native, &nx, &ny);
    eventX -= nx;
    eventY -= ny;

    graphene_point_t out { .x = 0, .y = 0 };
    if (auto p = graphene_point_t{ .x = float(eventX), .y = float(eventY) };
        !gtk_widget_compute_point(eventWidget, widget, &p, &out))
        return {};

    return glm::vec2 { out.x, out.y } * float(gdk_surface_get_scale(gtk_native_get_surface(native)));
}

static void aui_widget_init(AUIWidget* myWidget) {
    gtk_widget_set_focusable(GTK_WIDGET(myWidget), true);
    myWidget->imContext = gtk_im_multicontext_new();
    g_signal_connect(
        myWidget->imContext, "commit", G_CALLBACK(+[](GtkIMContext* im, const char* str, AUIWidget* widget) {
            for (auto c : AString(str)) {
                widget->renderingContext->window().onCharEntered(c);
            }
        }),
        myWidget);

    if (auto keyController = gtk_event_controller_key_new()) {
        auto keyPressed =
            [](GtkEventController* controller, guint keyval, guint keycode, GdkModifierType modifiers,
               AUIWidget* widget) -> gboolean {
            auto key = PlatformAbstractionGtk::inputFromNative2(keyval);
            gKeyStates[key] = true;
            widget->renderingContext->window().onKeyDown(key);
            return true;
        };
        auto keyReleased =
            [](GtkEventController* controller, guint keyval, guint keycode, GdkModifierType modifiers,
               AUIWidget* widget) -> gboolean {
            auto key = PlatformAbstractionGtk::inputFromNative2(keyval);
            gKeyStates[key] = false;
            widget->renderingContext->window().onKeyUp(key);
            return true;
        };
        g_signal_connect(keyController, "key-pressed", G_CALLBACK(+keyPressed), myWidget);
        g_signal_connect(keyController, "key-released", G_CALLBACK(+keyReleased), myWidget);
        gtk_event_controller_key_set_im_context(GTK_EVENT_CONTROLLER_KEY(keyController), myWidget->imContext);
        gtk_widget_add_controller(GTK_WIDGET(myWidget), keyController);
    }

    if (auto legacyController = gtk_event_controller_legacy_new()) {
        auto handler = [](GtkEventControllerLegacy* sender, GdkEvent* event, gpointer user_data) {
            auto* widget = AUI_WIDGET_WIDGET(user_data);
            auto mouseButton = [&] {
                auto index = gdk_button_event_get_button(event);
                switch (index) {
                    default:
                    case 1:
                        return AInput::LBUTTON;
                    case 3:
                        return AInput::RBUTTON;
                    case 2:
                        return AInput::CBUTTON;
                }
            };

            switch (gdk_event_get_event_type(event)) {
                case GDK_MOTION_NOTIFY: {
                    auto position = getEventPosition(event, GTK_WIDGET(widget));
                    widget->renderingContext->window().onPointerMove(position, APointerMoveEvent {});
                    return true;
                }

                case GDK_BUTTON_PRESS: {
                    auto position = getEventPosition(event, GTK_WIDGET(widget));

                    widget->renderingContext->window().onPointerPressed(APointerPressedEvent {
                      .position = position,
                      .pointerIndex = APointerIndex::button(mouseButton()),
                    });
                    return true;
                }

                case GDK_BUTTON_RELEASE: {
                    auto position = getEventPosition(event, GTK_WIDGET(widget));
                    widget->renderingContext->window().onPointerReleased(APointerReleasedEvent {
                      .position = position,
                      .pointerIndex = APointerIndex::button(mouseButton()),
                    });
                    return true;
                }

                case GDK_ENTER_NOTIFY: {
                    widget->renderingContext->window().onMouseEnter();
                    return true;
                }

                case GDK_LEAVE_NOTIFY: {
                    widget->renderingContext->window().onMouseLeave();
                    return true;
                }

                case GDK_SCROLL: {
                    auto delta = [&] {
                        double dx, dy;
                        gdk_scroll_event_get_deltas(event, &dx, &dy);
                        return glm::vec2 { dx, dy };
                    }();
                    if (gdk_scroll_event_get_unit(event) == GDK_SCROLL_UNIT_WHEEL) {
                        delta *= 120;
                    }
                    widget->renderingContext->window().onScroll({
                      .origin = widget->renderingContext->window().getMousePos(),
                      .delta = delta,
                      .kinetic = false,
                      .pointerIndex = APointerIndex::button(AInput::LBUTTON),
                    });
                    return true;
                }

                default:
                    return false;
            }
        };
        g_signal_connect(legacyController, "event", G_CALLBACK(+handler), myWidget);
        gtk_widget_add_controller(GTK_WIDGET(myWidget), legacyController);
    }

    /*
   gtk_widget_add_tick_callback(GTK_WIDGET(myWidget),
                                [](GtkWidget* widget, GdkFrameClock* frame_clock, gpointer user_data) -> gboolean {
                                    gtk_widget_queue_draw(widget);
                                    return G_SOURCE_CONTINUE; // Return TRUE to keep the callback active
                                },
                                nullptr, nullptr);
   // */
}

static void aui_widget_class_init(AUIWidgetClass* klass) {
    GTK_WIDGET_CLASS(klass)->realize = [](GtkWidget* widget) {
        GTK_WIDGET_CLASS(aui_widget_parent_class)->realize(widget);
        AUI_WIDGET_WIDGET(widget)->renderingContext->gtkRealize(widget);
    };
    GTK_WIDGET_CLASS(klass)->unrealize = [](GtkWidget* widget) {
        GTK_WIDGET_CLASS(aui_widget_parent_class)->unrealize(widget);
        AUI_WIDGET_WIDGET(widget)->renderingContext->gtkUnrealize(widget);
    };
    GTK_WIDGET_CLASS(klass)->snapshot = [](GtkWidget* widget, GtkSnapshot* snapshot) {
        GTK_WIDGET_CLASS(aui_widget_parent_class)->snapshot(widget, snapshot);
        AUI_WIDGET_WIDGET(widget)->renderingContext->gtkSnapshot(widget, snapshot);
    };

    G_OBJECT_CLASS(klass)->dispose = [](GObject* object) {
        auto widget = AUI_WIDGET_WIDGET(object);
        widget->~AUIWidget();
        G_OBJECT_CLASS(aui_widget_parent_class)->dispose(object);
    };

    G_OBJECT_CLASS(klass)->notify = [](GObject* object, GParamSpec* pspec) {
        auto name = std::string_view(pspec->name);
        if (name == "scale-factor") {
            auto widget = AUI_WIDGET_WIDGET(object);

            widget->renderingContext->gtkDoUnderContext([&] { widget->renderingContext->window().updateDpi(); });
            gtk_widget_queue_draw(GTK_WIDGET(widget));
        }
    };
}

GtkWidget* aui_widget_new(RenderingContextGtk& renderingContext) {
    auto widget = AUI_WIDGET_WIDGET(g_object_new(AUI_WIDGET_TYPE, nullptr));
    widget->renderingContext = &renderingContext;
    renderingContext.mAUIWidget = reinterpret_cast<GtkWidget*>(widget);
    return GTK_WIDGET(widget);
}

void RenderingContextGtk::gtkDoUnderContext(const std::function<void()>& callback) { callback(); }
