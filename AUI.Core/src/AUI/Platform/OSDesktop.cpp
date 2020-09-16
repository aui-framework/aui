#if !defined(__ANDROID__)
#include <AUI/api.h>
#include <AUI/Common/AStringVector.h>
#include <AUI/Thread/IEventLoop.h>


AUI_EXPORT int aui_main(int argc, char** argv, int(*aui_entry)(const AStringVector&)) {
    AStringVector args;
    for (int i = 0; i < argc; ++i) {
        args << argv[i];
    }
    int r = aui_entry(args);
    if (auto el = AThread::current()->getCurrentEventLoop()) {
        el->loop();
    }
    return r;
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