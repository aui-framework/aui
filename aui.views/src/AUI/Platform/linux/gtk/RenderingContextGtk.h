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

#pragma once

#include "gtk_types.h"
#include "AUI/Platform/ASurface.h"
#include "AUIWidget.h"

class RenderingContextGtk {
public:
    friend aui::gtk4_fake::GtkWidget* aui_widget_new(RenderingContextGtk& renderingContext);
    explicit RenderingContextGtk(ASurface& window) : mWindow(window) {}
    virtual ~RenderingContextGtk() = default;

    virtual void gtkRealize(aui::gtk4_fake::GtkWidget* widget) = 0;
    virtual void gtkSnapshot(aui::gtk4_fake::GtkWidget* widget, aui::gtk4_fake::GtkSnapshot* snapshot) = 0;
    virtual void gtkUnrealize(aui::gtk4_fake::GtkWidget* widget) = 0;
    virtual void gtkDoUnderContext(const std::function<void()>& callback);

    ASurface& window() const { return mWindow; }
    aui::gtk4_fake::GtkWidget* auiWidget() const { return mAUIWidget; }


protected:
    ASurface& mWindow;
    aui::gtk4_fake::GtkWidget* mAUIWidget = nullptr;
};
