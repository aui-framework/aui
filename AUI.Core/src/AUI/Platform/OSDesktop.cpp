#if !defined(__ANDROID__)
#include <AUI/api.h>
#include <AUI/Common/AStringVector.h>
#include "Dll.h"


#if defined(WIN32)
AUI_EXPORT int aui_main(int argc, char** argv, int(*aui_entry)(const AStringVector&)) {
#else
AUI_IMPORT int aui_entry(const AStringVector& args);
AUI_EXPORT int main(int argc, char** argv) {
#endif
    AStringVector args;
    for (int i = 0; i < argc; ++i) {
        args << argv[i];
    }
    return aui_entry(args);
}
#if defined(__WIN32)
#include <Windows.h>
AUI_EXPORT int __stdcall WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR     lpCmdLine,
	int       nShowCmd
) {
    return main(0, nullptr);
}
#endif

#endif