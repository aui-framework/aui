#if !defined(__ANDROID__)
#include <AUI/api.h>
#include <AUI/Common/AStringVector.h>
#include <AUI/Thread/IEventLoop.h>
#include <AUI/Common/AException.h>
#include <AUI/Util/AError.h>


AUI_EXPORT int aui_main(int argc, char** argv, int(*aui_entry)(const AStringVector&)) {
    AStringVector args;
    for (int i = 0; i < argc; ++i) {
        args << argv[i];
    }
    int r = -1;
    try {
        r = aui_entry(args);
        if (auto el = AThread::current()->getCurrentEventLoop()) {
            el->loop();
        }
    } catch (const AException& e) {
        AError::handle(e, "uncaught exception");
    } catch (...) {
        ALogger::err("Caught exception of unknown type");
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