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

// gio goes first: AUI headers define the `signals` macro which breaks it.
#include <gio/gio.h>
#include "GtkMenuProvider.h"
#include "PlatformAbstractionGtk.h"
#include "RenderingContextGtk.h"
#include "gtk_functions.h"
#include <AUI/Action/AMenu.h>
#include <AUI/Action/AShortcut.h>
#include <AUI/Image/AImage.h>
#include <AUI/Image/IDrawable.h>
#include <AUI/Platform/AInput.h>
#include <AUI/Platform/ASurface.h>
#include <AUI/Platform/linux/AGlibPtr.h>

#include <cmath>
#include <functional>

using namespace aui::gtk4_fake;

namespace {

constexpr auto ACTION_GROUP = "aui-menu";
constexpr auto CALLBACK_KEY = "aui-callback";
constexpr int ICON_SIZE = 16;   // logical pixels

/**
 * @brief GtkPopoverMenu labels treat underscore as a mnemonic prefix.
 */
std::string toLabel(const AString& s) {
    std::string result;
    for (char c : s.toStdString()) {
        result += c;
        if (c == '_') {
            result += '_';
        }
    }
    return result;
}

/**
 * @brief Converts AShortcut to GTK accelerator string (i.e., `<Control><Shift>s`); empty if it can't be represented.
 */
std::string toAccelerator(const AShortcut& shortcut) {
    if (shortcut.empty()) {
        return {};
    }
    std::string modifiers;
    std::string key;
    for (const auto k : shortcut.getKeys()) {
        switch (k) {
            case AInput::LCONTROL:
            case AInput::RCONTROL:
                modifiers += "<Control>";
                break;
            case AInput::LSYSTEM:
            case AInput::RSYSTEM:
                modifiers += "<Super>";
                break;
            case AInput::LSHIFT:
            case AInput::RSHIFT:
                modifiers += "<Shift>";
                break;
            case AInput::LALT:
            case AInput::RALT:
                modifiers += "<Alt>";
                break;
            default: {
                auto name = AInput::getName(k);
                // single letters are keysyms in lowercase; the rest (F5, Return, ...) are capitalized.
                key = name.length() == 1 ? name.lowercase().toStdString() : name.toStdString();
                break;
            }
        }
    }
    if (key.empty()) {
        return {};
    }
    return modifiers + key;
}

/**
 * @return GdkTexture (it implements GIcon) or null.
 */
AGlibPtr<GIcon> toIcon(const _<IDrawable>& drawable, int scale) {
    if (!drawable) {
        return {};
    }
    const int size = ICON_SIZE * scale;
    AImage rasterized = drawable->rasterize({ size, size });
    if (rasterized.buffer().empty()) {
        return {};
    }
    auto rgba = rasterized.convert(APixelFormat::RGBA | APixelFormat::BYTE);
    auto bytes = g_bytes_new(rgba.buffer().data(), rgba.buffer().size());
    auto texture = gdk_memory_texture_new(rgba.width(), rgba.height(), GDK_MEMORY_R8G8B8A8, bytes, size_t(rgba.width()) * 4);
    g_bytes_unref(bytes);
    return AGlibPtr<GIcon>(reinterpret_cast<GIcon*>(texture));
}

struct Builder {
    GSimpleActionGroup* group;
    int scale;
    unsigned nextId = 0;
};

AGlibPtr<GMenu> buildMenu(const AVector<AMenuItem>& items, Builder& builder) {
    // GMenu has no separators; items are divided by sections instead.
    AGlibPtr<GMenu> root(g_menu_new());
    AGlibPtr<GMenu> section(g_menu_new());
    auto flush = [&] {
        if (g_menu_model_get_n_items(G_MENU_MODEL(section.value())) > 0) {
            g_menu_append_section(root.value(), nullptr, G_MENU_MODEL(section.value()));
        }
        section = g_menu_new();
    };

    for (const auto& item : items) {
        switch (item.type) {
            case AMenu::SEPARATOR:
                flush();
                break;

            case AMenu::SUBLIST: {
                auto sub = buildMenu(item.subItems, builder);
                AGlibPtr<GMenuItem> mi(g_menu_item_new_submenu(toLabel(item.name).c_str(), G_MENU_MODEL(sub.value())));
                if (auto icon = toIcon(item.icon, builder.scale); icon) {
                    g_menu_item_set_icon(mi.value(), icon.value());
                }
                g_menu_append_item(section.value(), mi.value());
                break;
            }

            case AMenu::SINGLE: {
                const auto actionName = "i" + std::to_string(builder.nextId++);
                AGlibPtr<GSimpleAction> action(g_simple_action_new(actionName.c_str(), nullptr));
                g_simple_action_set_enabled(action.value(), item.enabled);
                g_object_set_data_full(
                    G_OBJECT(action.value()), CALLBACK_KEY, new std::function<void()>(item.onAction),
                    +[](gpointer p) { delete static_cast<std::function<void()>*>(p); });
                g_signal_connect(
                    action.value(), "activate",
                    G_CALLBACK(+[](GSimpleAction* self, GVariant*, gpointer) {
                        auto cb = static_cast<std::function<void()>*>(g_object_get_data(G_OBJECT(self), CALLBACK_KEY));
                        if (cb && *cb) {
                            (*cb)();
                        }
                    }),
                    nullptr);
                g_action_map_add_action(G_ACTION_MAP(builder.group), G_ACTION(action.value()));

                const auto detailed = std::string(ACTION_GROUP) + "." + actionName;
                AGlibPtr<GMenuItem> mi(g_menu_item_new(toLabel(item.name).c_str(), detailed.c_str()));
                if (const auto accel = toAccelerator(item.shortcut); !accel.empty()) {
                    // display only; the shortcut itself is handled by AUI.
                    g_menu_item_set_attribute(mi.value(), "accel", "s", accel.c_str());
                }
                if (auto icon = toIcon(item.icon, builder.scale); icon) {
                    g_menu_item_set_icon(mi.value(), icon.value());
                }
                g_menu_append_item(section.value(), mi.value());
                break;
            }
        }
    }
    flush();
    return root;
}

double surfaceScale(GtkWidget* widget) {
    if (auto native = gtk_widget_get_native(widget)) {
        if (auto surface = gtk_native_get_surface(native)) {
            return gdk_surface_get_scale(surface);
        }
    }
    return 1.0;
}

}   // namespace

GtkMenuProvider::GtkMenuProvider() = default;

GtkMenuProvider::~GtkMenuProvider() { closeMenu(); }

void GtkMenuProvider::createMenu(const AVector<AMenuItem>& vector) {
    closeMenu();

    auto surface = ASurface::current();
    auto context = surface ? dynamic_cast<RenderingContextGtk*>(surface->getRenderingContext().get()) : nullptr;
    if (!context || !context->auiWidget()) {
        mFallback.createMenu(vector);
        return;
    }
    auto parent = context->auiWidget();

    // AUI works in physical pixels, GTK in logical ones.
    const double scale = surfaceScale(parent);

    AGlibPtr<GSimpleActionGroup> group(g_simple_action_group_new());
    Builder builder { group.value(), int(std::ceil(scale)) };
    auto model = buildMenu(vector, builder);

    auto popover = gtk_popover_menu_new_from_model(G_MENU_MODEL(model.value()));
    g_object_ref_sink(popover);
    gtk_widget_insert_action_group(popover, ACTION_GROUP, G_ACTION_GROUP(group.value()));
    gtk_widget_set_parent(popover, parent);

    auto popoverHandle = reinterpret_cast<GtkPopover*>(popover);
    gtk_popover_set_has_arrow(popoverHandle, false);
    gtk_widget_set_halign(popover, GTK_ALIGN_START);

    const auto mouse = surface->getMousePos();
    GdkRectangle rect { .x = int(std::floor(double(mouse.x) / scale)),
                        .y = int(std::floor(double(mouse.y) / scale)),
                        .width = 1,
                        .height = 1 };
    gtk_popover_set_pointing_to(popoverHandle, &rect);

    g_signal_connect(popover, "closed", G_CALLBACK(onClosed), this);
    mPopover = popover;
    gtk_popover_popup(popoverHandle);
}

void GtkMenuProvider::closeMenu() {
    if (mFallback.isOpen()) {
        mFallback.closeMenu();
    }
    if (!mPopover) {
        return;
    }
    auto popover = static_cast<GtkWidget*>(mPopover);
    detach();
    gtk_popover_popdown(reinterpret_cast<GtkPopover*>(popover));
}

bool GtkMenuProvider::isOpen() { return mPopover != nullptr || mFallback.isOpen(); }

bool GtkMenuProvider::isSupported() {
    return dynamic_cast<PlatformAbstractionGtk*>(IPlatformAbstraction::currentSafe()) != nullptr;
}

void GtkMenuProvider::onClosed(void*, GtkMenuProvider* self) { self->detach(); }

void GtkMenuProvider::detach() {
    if (!mPopover) {
        return;
    }
    auto popover = static_cast<GtkWidget*>(mPopover);
    mPopover = nullptr;
    g_signal_handlers_disconnect_by_data(popover, this);

    // the popover closes right when an item is activated, probably before or in the middle of its action callback;
    // destroying the popover (and the action group that owns the callback) must wait until that is over.
    g_idle_add(
        +[](gpointer p) -> gboolean {
            auto widget = static_cast<GtkWidget*>(p);
            gtk_widget_unparent(widget);
            g_object_unref(widget);
            return G_SOURCE_REMOVE;
        },
        popover);
}
