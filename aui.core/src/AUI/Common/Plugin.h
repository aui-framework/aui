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
