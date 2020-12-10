#if defined(_WIN32)
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

#include "AUI/Common/Plugin.h"
#include "AUI/Util/BuiltinFiles.h"
#include "Render/Stylesheet.h"
#include <AUI/Logging/ALogger.h>



struct initialize
{
    initialize() {
#ifndef ANDROID
        try {
            aui::importPlugin("svg");
        } catch (const AException& e) {
            ALogger::warn("Could not load Svg plugin:" + e.getMessage());
        }
        try {
            aui::importPlugin("image");
        } catch (const AException& e) {
            ALogger::warn("Could not load Image plugin:" + e.getMessage());
        }
#endif
#ifdef _WIN32
        typedef BOOL(WINAPI *SetProcessDpiAwarenessContext_t)(HANDLE);
        auto SetProcessDpiAwarenessContext = (SetProcessDpiAwarenessContext_t)GetProcAddress(GetModuleHandleA("User32.dll"), "SetProcessDpiAwarenessContext");

        if (SetProcessDpiAwarenessContext) {
            // DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2
            SetProcessDpiAwarenessContext((HANDLE) -4);
        }
#endif
    }
} init;