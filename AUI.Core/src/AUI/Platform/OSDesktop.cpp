#if !defined(__ANDROID__)
#include <AUI/api.h>
#include <AUI/Common/AStringVector.h>
#include <AUI/Thread/IEventLoop.h>
#include <AUI/Common/AException.h>
#include <AUI/Util/AError.h>

#if defined(_WIN32)
#include <windows.h>
#endif


AUI_EXPORT int aui_main(int argc, char** argv, int(*aui_entry)(const AStringVector&)) {
    AStringVector args;
#if defined(_WIN32)
    if (argc == 0) {
        // убрать кавычки
        AString argsRaw = GetCommandLineW();
        bool wrappedWithQuots = false;
        AString currentArg;
        for (auto& c : argsRaw) {
            switch (c) {
                case '\"':
                    wrappedWithQuots = !wrappedWithQuots;
                    break;

                case ' ':
                    if (!wrappedWithQuots) {
                        args << std::move(currentArg);
                        currentArg = {};
                        assert(currentArg.empty());
                        break;
                    }
                default:
                    currentArg += c;
            }
        }
        if (!currentArg.empty()) {
            args << std::move(currentArg);
        }
    }
#endif
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
#endif