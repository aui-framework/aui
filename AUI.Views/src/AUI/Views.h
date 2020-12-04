#pragma once
#pragma warning(disable: 4251)

#include "AUI/api.h"
#include "AUI/Autumn/Autumn.h"
#include "AUI/Util/Factory.h"
#include "Util/IShadingEffect.h"


namespace AViews
{
	API_AUI_VIEWS void mergeAutumnObjects(const AMap<AString, _<Factory<IShadingEffect>>>& data);
	inline void mergeAutumnObjects()
	{
		auto res = Autumn::detail::storageMap<Factory<IShadingEffect>>();
		mergeAutumnObjects(res);
	}
}
#if defined(_WIN32)
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' "\
"version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif