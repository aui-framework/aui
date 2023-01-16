// AUI Framework - Declarative UI toolkit for modern C++17
// Copyright (C) 2020-2023 Alex2772
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

#if AUI_PLATFORM_WIN
#include <windows.h>

#endif

#include <AUI/Url/AUrl.h>
#include <AUI/Curl/ACurl.h>


/**
 * @defgroup curl aui::curl
 * @brief aui::curl is a http(s)/ftp driver based on cURL.
 */

/*
struct main {
    main() {
        AUrl::registerResolver("http", [](const AUrl& u) {
            return _new<ACurl>(u.getFull());
        });
        AUrl::registerResolver("https", [](const AUrl& u) {
            return _new<ACurl>(u.getFull());
        });
    }
} m;*/