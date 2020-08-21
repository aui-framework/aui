#include "AUI/Common/Plugin.h"

#include "AString.h"
#include "Set.h"
#include "AUI/Platform/Dll.h"

void aui::importPlugin(const AString& name)
{
    auto n = name;
    n[0] = toupper(n[0]);
	auto path = AString::path(std::filesystem::path(n.toStdString()).filename());
	
	static Set<AString> importedPlugins;
	if (!importedPlugins.contains(path))
	{
		importedPlugins << path;
		Dll::load("AUI." + path)->getProcAddress<void()>("aui_plugin_init")();
	}
	
}
