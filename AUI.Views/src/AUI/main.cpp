#ifdef _WIN32
#include <Windows.h>


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
#else
#include <gtk/gtk.h>
#endif

#include "AUI/Common/Plugin.h"
#include "AUI/Util/BuiltinFiles.h"
#include "Render/Stylesheet.h"

#include "assets_views.h"

struct initialize
{
    initialize() {
#ifdef __linux
        gtk_init(nullptr, nullptr);
#endif

        aui::importPlugin("Svg");

        BuiltinFiles f;
        ByteBuffer b(AUI_PACKED_assets_view, sizeof(AUI_PACKED_assets_view));
        f.loadBuffer(b);

        Stylesheet::instance().load(f.open("assets/win/style.css"));
    }
} init;