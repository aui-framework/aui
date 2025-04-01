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

#include <gtk/gtk.h>

#include "Gtk.h"
#include "AUI/Util/kAUI.h"

namespace aui::detail {
extern int argc;
extern char** argv;
}

void aui::detail::gtk::requires_gtk() {
    do_once { gtk_init(&aui::detail::argc, &aui::detail::argv); }
}

void aui::detail::gtk::main_iterations() {
    while (gtk_events_pending()) {
        gtk_main_iteration();
    }
}
