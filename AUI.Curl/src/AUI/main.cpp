#ifdef _WIN32
#include <windows.h>


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
#endif

#include <AUI/Url/AUrl.h>
#include <AUI/Curl/ACurl.h>

struct main {
    main() {
        AUrl::registerResolver("http", [](const AUrl& u) {
            return _new<ACurl>(u.getFull());
        });
        AUrl::registerResolver("https", [](const AUrl& u) {
            return _new<ACurl>(u.getFull());
        });
    }
} m;