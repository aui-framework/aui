// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2023 Alex2772
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

//
// Created by Alex2772 on 3/7/2022.
//

#include <AUI/Platform/AStacktrace.h>
#include <Windows.h>
#include <dbghelp.h>
#include <AUI/IO/APath.h>
#include <mutex>
#include "AUI/Thread/AMutex.h"

struct Win32SymService {
    HANDLE process;
    AMutex mutex;

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
    assert(("too many", maxFrames <= std::size(backtrace)));
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
