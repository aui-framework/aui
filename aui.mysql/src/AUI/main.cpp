// AUI Framework - Declarative UI toolkit for modern C++17
// Copyright (C) 2020-2024 Alex2772 and Contributors
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

#include <mysql.h>
#include "AUI/Common/Plugin.h"
#include "AUI/Data/ASqlDatabase.h"
#include "AMysql.h"

AUI_PLUGIN_ENTRY {
    mysql_library_init(0, nullptr, nullptr);
    ASqlDatabase::registerDriver(_new<AMysql>());
}