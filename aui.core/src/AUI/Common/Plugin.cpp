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

#include <AUI/IO/APath.h>
#include "AUI/Common/Plugin.h"

#include "AString.h"
#include <AUI/Traits/strings.h>
#include "ASet.h"
#include "AUI/Platform/AProgramModule.h"
#include "Plugin.h"

void aui::importPlugin(const AString& name)
{
	AString filename = APath(name).filename();
	
	static ASet<AString> importedPlugins;
	if (!importedPlugins.contains(filename))
	{
		importedPlugins << filename;
		aui::importPluginPath("aui." + filename.lowercase());
	}
	
}

void aui::importPluginPath(const APath& path) {
	if (auto module = AProgramModule::load(path)) {
        if (auto proc = module->getProcAddress<void()>("aui_plugin_init")) {
            proc();
            return;
        }
        throw AException("Module {} does not contain plugin entry point"_format(path));
    }
}
