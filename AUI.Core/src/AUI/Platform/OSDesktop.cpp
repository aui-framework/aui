#if !defined(__ANDROID__)
#include <AUI/api.h>
#include <AUI/Common/AStringVector.h>

AUI_IMPORT int aui_entry(const AStringVector& args);

AUI_EXPORT int main(int argc, char** argv) {
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