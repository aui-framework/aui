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

#include <dlfcn.h>
#include "PlatformAbstractionAdw.h"
#include "adw_functions.h"
namespace aui::adw1_fake {
extern void* handle;
}

using namespace aui::gtk4_fake;
using namespace aui::adw1_fake;

PlatformAbstractionAdw::PlatformAbstractionAdw() {
    handle = dlopen("libadwaita-1.so", RTLD_LAZY | RTLD_GLOBAL);
    if (!handle) {
        throw AException("failed to load libadwaita-1.so: {}"_format(dlerror()));
    }
}

void PlatformAbstractionAdw::init() {
    mApplication = G_APPLICATION(adw_application_new(nullptr, static_cast<GApplicationFlags>(0)));
    PlatformAbstractionGtk::init();

    // TODO at the moment aui does not really support colors schemes, so we force light mode
    adw_style_manager_set_color_scheme(adw_application_get_style_manager(ADW_APPLICATION(mApplication)),
                                       ADW_COLOR_SCHEME_FORCE_LIGHT);
}

void PlatformAbstractionAdw::windowManagerInitNativeWindow(const IRenderingContext::Init& init) {
    auto window = GTK_WINDOW(adw_application_window_new(GTK_APPLICATION(*mApplication)));
    windowManagerInitCommon(init, window);
    auto toolbar = adw_toolbar_view_new();
    auto header = adw_header_bar_new();
    adw_toolbar_view_add_top_bar(ADW_TOOLBAR_VIEW(toolbar), header);
    adw_toolbar_view_set_top_bar_style(ADW_TOOLBAR_VIEW(toolbar), ADW_TOOLBAR_RAISED_BORDER);
    adw_toolbar_view_set_extend_content_to_top_edge(ADW_TOOLBAR_VIEW(toolbar), false);
    adw_toolbar_view_set_content(ADW_TOOLBAR_VIEW(toolbar), windowManagerInitGtkBox(init));
    adw_application_window_set_content(ADW_APPLICATION_WINDOW(window), toolbar);
}
