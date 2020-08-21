#include "Platform.h"
#include "AUI/Common/AString.h"

#ifdef _WIN32
#include <Windows.h>

AString Platform::getFontPath(const AString& font)
{
    if (std::filesystem::is_regular_file(font.toStdString()))
        return font;
	return "C:/Windows/Fonts/" + font + ".ttf";
}

void Platform::playSystemSound(Sound s)
{
	switch (s)
	{
	case S_QUESTION:
		PlaySound(L"SystemQuestion", nullptr, SND_ASYNC);
		break;
		
	case S_ASTERISK:
		PlaySound(L"SystemAsterisk", nullptr, SND_ASYNC);
		break;
		
	}
}

float Platform::getDpiRatio()
{
	return GetDpiForSystem() / 96.f;
}
#else

AString Platform::getFontPath(const AString& font)
{
    if (std::filesystem::is_regular_file(font.toStdString()))
        return font;

    return "/usr/share/fonts/truetype/" + font;
}

void Platform::playSystemSound(Sound s)
{
    // unsupported
}

float Platform::getDpiRatio()
{
    return 1.f;
}

#endif