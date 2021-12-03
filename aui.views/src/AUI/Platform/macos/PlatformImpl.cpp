/**
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
 */

#include "AUI/Platform/Platform.h"
#include "AUI/Common/AString.h"
#include "AUI/IO/APath.h"
#include <AUI/Util/kAUI.h>

#if AUI_PLATFORM_WIN
#include <Windows.h>
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
    typedef UINT(WINAPI *GetDpiForSystem_t)();
    static auto GetDpiForSystem = (GetDpiForSystem_t)GetProcAddress(GetModuleHandleA("User32.dll"), "GetDpiForSystem");
	if (GetDpiForSystem) {
        return GetDpiForSystem() / 96.f;
    }
	return 1.f;
}
#else

#if AUI_PLATFORM_ANDROID
#include <AUI/Platform/OSAndroid.h>

#elif AUI_PLATFORM_APPLE
#else

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xresource.h>

#endif


void Platform::playSystemSound(Sound s)
{
    // unsupported
}

#ifdef __linux
#if !(AUI_PLATFORM_ANDROID)
extern Display* gDisplay;
void ensureXLibInitialized();
#endif
#endif

float Platform::getDpiRatio()
{
#if AUI_PLATFORM_ANDROID
    return AAndroid::getDpiRatio();

#elif AUI_PLATFORM_APPLE
    // TODO apple
    return 1.f;
#else
    ensureXLibInitialized();
    char *resourceString = XResourceManagerString(gDisplay);
    XrmDatabase db;
    XrmValue value;
    char *type = NULL;
    float dpi = 1.f;

    do_once {
        XrmInitialize();
    };

    db = XrmGetStringDatabase(resourceString);

    if (resourceString) {
        if (XrmGetResource(db, "Xft.dpi", "String", &type, &value)) {
            if (value.addr) {
                dpi = atof(value.addr) / 96.f;
            }
        }
    }

    return dpi;
#endif
}

#endif