/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#if AUI_PLATFORM_WIN
#include <windows.h>

#endif

#include <AUI/Url/AUrl.h>
#include <AUI/Curl/ACurl.h>


struct ACurlInit {
    ACurlInit() {
        AUrl::registerResolver("http", [](const AUrl& u) {
            return ACurl::Builder(u.full()).toInputStream();
        });
        AUrl::registerResolver("https", [](const AUrl& u) {
            return ACurl::Builder(u.full()).toInputStream();
        });
    }
} _acurlinit;