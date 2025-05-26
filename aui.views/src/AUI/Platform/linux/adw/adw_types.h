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

#include <AUI/Platform/linux/gtk/gtk_types.h>
#include <AUI/Platform/linux/gtk/gtk_functions.h>

namespace aui::adw1_fake {
using namespace aui::gtk4_fake;
extern "C" {
typedef enum {
    ADW_COLOR_SCHEME_DEFAULT,
    ADW_COLOR_SCHEME_FORCE_LIGHT,
    ADW_COLOR_SCHEME_PREFER_LIGHT,
    ADW_COLOR_SCHEME_PREFER_DARK,
    ADW_COLOR_SCHEME_FORCE_DARK,
} AdwColorScheme;

using AdwApplication = struct _AdwApplication;
using AdwStyleManager = struct _AdwStyleManager;
#define ADW_TYPE_APPLICATION (adw_application_get_type())

G_DECLARE_DERIVABLE_TYPE(AdwApplication, adw_application, ADW, APPLICATION, GtkApplication)


#define ADW_TYPE_APPLICATION_WINDOW (adw_application_window_get_type())
G_DECLARE_DERIVABLE_TYPE (AdwApplicationWindow, adw_application_window, ADW, APPLICATION_WINDOW, GtkApplicationWindow)


#define ADW_TYPE_TOOLBAR_VIEW (adw_toolbar_view_get_type())

typedef enum {
    ADW_TOOLBAR_FLAT,
    ADW_TOOLBAR_RAISED,
    ADW_TOOLBAR_RAISED_BORDER,
} AdwToolbarStyle;

G_DECLARE_FINAL_TYPE (AdwToolbarView, adw_toolbar_view, ADW, TOOLBAR_VIEW, GtkWidget)

}
}