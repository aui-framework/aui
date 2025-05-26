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

#include <glib-object.h>
#include "gtk_types.h"

class RenderingContextGtk;

// Gtk world representation of AUI.
// Implementation is based on gtkglarea and https://gitlab.gnome.org/GNOME/gtk/-/merge_requests/8503
aui::gtk4_fake::GtkWidget* aui_widget_new(RenderingContextGtk& renderingContext);


