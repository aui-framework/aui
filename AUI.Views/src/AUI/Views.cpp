#include "Views.h"

API_AUI_VIEWS void AViews::mergeAutumnObjects(const AMap<AString, _<Factory<IShadingEffect>>>& data)
{
	Autumn::detail::storageMap<Factory<IShadingEffect>>() = data;
}
