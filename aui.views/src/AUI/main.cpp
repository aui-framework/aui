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

#if AUI_PLATFORM_WIN
#include <windows.h>
#include <AUI/Url/AUrl.h>


BOOL WINAPI DllMain(
	HINSTANCE hinstDLL,  // handle to DLL module
	DWORD fdwReason,     // reason for calling function
	LPVOID lpReserved)  // reserved
{
	// Perform actions based on the reason for calling.
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
		// Initialize once for each new process.
		// Return FALSE to fail DLL load.
		break;

	case DLL_THREAD_ATTACH:
		// Do thread-specific initialization.
		break;

	case DLL_THREAD_DETACH:
		// Do thread-specific cleanup.
		break;

	case DLL_PROCESS_DETACH:
		// Perform any necessary cleanup.
		break;
	}
	return TRUE;  // Successful DLL_PROCESS_ATTACH.
}

#elif defined(ANDROID)
#else
#include <gtk/gtk.h>
#include <AUI/Logging/ALogger.h>

#endif


#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include "AUI/Common/Plugin.h"
#include <AUI/Logging/ALogger.h>
#include <AUI/Image/JpgImageLoader.h>
#include <AUI/Image/PngImageLoader.h>
#include <AUI/Image/AImageLoaderRegistry.h>
#include <AUI/Image/SvgImageLoader.h>


struct initialize
{
    initialize() {
        AImageLoaderRegistry::inst().registerImageLoader(_new<SvgImageLoader>());
        AImageLoaderRegistry::inst().registerImageLoader(_new<PngImageLoader>());
        AImageLoaderRegistry::inst().registerImageLoader(_new<JpgImageLoader>());

#if AUI_PLATFORM_WIN
#ifndef AUI_DISABLE_HIDPI
        typedef BOOL(WINAPI *SetProcessDpiAwarenessContext_t)(HANDLE);
        auto SetProcessDpiAwarenessContext = (SetProcessDpiAwarenessContext_t)GetProcAddress(GetModuleHandleA("User32.dll"), "SetProcessDpiAwarenessContext");

        if (SetProcessDpiAwarenessContext) {
            // DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2
            SetProcessDpiAwarenessContext((HANDLE) -4);
        }
#endif
#endif
    }
} init;