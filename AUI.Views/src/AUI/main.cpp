#if defined(_WIN32)
#include <Windows.h>
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
#endif

#include "AUI/Common/Plugin.h"
#include "AUI/Util/BuiltinFiles.h"
#include "Render/Stylesheet.h"

struct initialize
{
    initialize() {
#ifndef ANDROID
        aui::importPlugin("Svg");
#endif
    }
} init;