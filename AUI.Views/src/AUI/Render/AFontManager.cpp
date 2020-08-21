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
	return _new<AFont>(this, Platform::getFontPath(name));
}

_<AFont> AFontManager::getDefault() {
#ifdef _WIN32
	return get("segoeui");
#else
	return get("ubuntu/Ubuntu-R.ttf");
#endif
}