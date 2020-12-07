#include <AUI/Url/AUrl.h>
#include "AFontManager.h"
#include "AUI/Platform/Platform.h"
#include "FreeType.h"

AFontManager::AFontManager() :
	mFreeType(_new<FreeType>())
{
}

AFontManager::~AFontManager() {
	mItems.clear();
}

_<AFont> AFontManager::newItem(const AString& name) {
    if (name.contains(":")) {
        // url
        return _new<AFont>(this, AUrl(name));
    }

	return _new<AFont>(this, Platform::getFontPath(name));
}

_<AFont> AFontManager::getDefault() {
#if defined(_WIN32)
	return get("segoeui");
#elif defined(ANDROID)
	return get(":and/font/Roboto-Medium.ttf");
#else
	return get("ubuntu/Ubuntu-R.ttf");
#endif
}

AFontManager& AFontManager::inst() {
    static AFontManager f;
    return f;
}
