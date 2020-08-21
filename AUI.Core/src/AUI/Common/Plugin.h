#pragma once

#include "AUI/Core.h"

#define AUI_PLUGIN_ENTRY extern "C" AUI_EXPORT void aui_plugin_init()

class AString;

namespace aui
{
	API_AUI_CORE void importPlugin(const AString& name);
};
