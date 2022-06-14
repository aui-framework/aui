//
// Created by Alex2772 on 3/7/2022.
//

#include <AUI/Platform/AStacktrace.h>
#include <Windows.h>
#include <AUI/Traits/arrays.h>
#include <dbghelp.h>
#include <AUI/IO/APath.h>
#include <mutex>

struct Win32SymService {
    HANDLE process;
    std::mutex mutex;

    Win32SymService() noexcept {
        process = GetCurrentProcess();
        SymInitialize(process, nullptr, true);
    }
};

static Win32SymService& symService() noexcept {
    static Win32SymService symService;
    return symService;
}

AStacktrace AStacktrace::capture(unsigned skipFrames, unsigned maxFrames) noexcept {
    symService();
    void* backtrace[128];
    assert(("too many", maxFrames <= aui::array_length(backtrace)));
    std::size_t entryCount = CaptureStackBackTrace(skipFrames + 1, maxFrames, backtrace, nullptr);

    AVector<Entry> entries;
    entries.reserve(entryCount);
    for (std::size_t i = 0; i < entryCount; ++i) {
        entries << Entry{backtrace[i]};
    }

    return AStacktrace(std::move(entries));
}


void AStacktrace::resolveSymbolsIfNeeded() const noexcept {
    if (!mSymbolNamesResolved) {
        mSymbolNamesResolved = true;

        DWORD dwDisplacement;
        auto symbolInfo = reinterpret_cast<SYMBOL_INFO*>(malloc(sizeof(SYMBOL_INFO) + 256));
        symbolInfo->SizeOfStruct = sizeof(SYMBOL_INFO);
        symbolInfo->MaxNameLen = 255;

        IMAGEHLP_LINE imageHlp;
        imageHlp.SizeOfStruct = sizeof(IMAGEHLP_LINE);

        std::unique_lock lock(symService().mutex);  // windbg is singlethreaded
        SymSetOptions(SYMOPT_LOAD_LINES);
        for (auto& symbol : mEntries) {
            if (symbol.mPtr) {
                if (SymFromAddr(symService().process, reinterpret_cast<std::uintptr_t>(symbol.mPtr), nullptr,
                                symbolInfo)) {
                    symbol.mFunctionName = AString(std::string(symbolInfo->Name, symbolInfo->NameLen));
                }
                if (SymGetLineFromAddr(symService().process, reinterpret_cast<std::uintptr_t>(symbol.mPtr), &dwDisplacement,
                                       &imageHlp)) {
                    symbol.mFileName = APath(imageHlp.FileName).filename();
                    symbol.mLineNumber = imageHlp.LineNumber;
                }
            }
        }
        free(symbolInfo);
    }
}
