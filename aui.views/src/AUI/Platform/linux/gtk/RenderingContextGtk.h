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

#include <gtk/gtk.h>
#include "AUI/Platform/AWindowBase.h"
#include "AUIWidget.h"

class RenderingContextGtk {
public:
    friend AUIWidget* aui_widget_new(RenderingContextGtk& renderingContext);
    explicit RenderingContextGtk(AWindowBase& window) : mWindow(window) {}
    virtual ~RenderingContextGtk() = default;

    virtual void gtkRealize(GtkWidget* widget) = 0;
    virtual void gtkSnapshot(GtkWidget* widget, GtkSnapshot* snapshot) = 0;
    virtual void gtkUnrealize(GtkWidget* widget) = 0;
    virtual void gtkDoUnderContext(const std::function<void()>& callback);

    AWindowBase& window() const { return mWindow; }
    AUIWidget* auiWidget() const { return mAUIWidget; }


protected:
    AWindowBase& mWindow;
    AUIWidget* mAUIWidget = nullptr;
};
