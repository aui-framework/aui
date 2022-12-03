// AUI Framework - Declarative UI toolkit for modern C++17
// Copyright (C) 2020-2022 Alex2772
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

#pragma once

#include "AUI/Core.h"
#include <AUI/IO/APath.h>

#ifdef AUI_STATIC
#ifdef _AUI_PLUGIN_ENTRY_N
#define AUI_PLUGIN_ENTRY struct _AUI_PLUGIN_ENTRY_N{ _AUI_PLUGIN_ENTRY_N() noexcept; } _AUI_PLUGIN_ENTRY_N ## v; _AUI_PLUGIN_ENTRY_N :: _AUI_PLUGIN_ENTRY_N () noexcept
#else
#define AUI_PLUGIN_ENTRY struct fail { fail(); static_assert(false, "usage of AUI_PLUGIN_ENTRY without PLUGIN flag in AUI_MODULE"); }; fail::fail()
#endif
#else
#define AUI_PLUGIN_ENTRY extern "C" AUI_EXPORT void aui_plugin_init()
#endif

class AString;

namespace aui
{
    /**
     * @brief imports plugin by it's name.
     * @param name plugin name
     * @ingroup core
     */
	API_AUI_CORE void importPlugin(const AString& name);

    /**
     * @brief imports plugin by it's path.
     * @param path plugin path
     * @ingroup core
     */
	API_AUI_CORE void importPluginPath(const APath& path);
};
