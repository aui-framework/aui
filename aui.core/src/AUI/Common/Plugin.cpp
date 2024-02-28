// AUI Framework - Declarative UI toolkit for modern C++20
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
