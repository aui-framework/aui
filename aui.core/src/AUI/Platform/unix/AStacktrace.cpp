// AUI Framework - Declarative UI toolkit for modern C++17
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

#if AUI_USE_BACKTRACE
#include <AUI/Platform/AStacktrace.h>
#include <AUI/Platform/AProcess.h>
#include <AUI/IO/APath.h>
#include <execinfo.h>
#include <backtrace.h>
#include "AUI/Logging/ALogger.h"
#include <cxxabi.h>

static void aui_backtrace_error_callback(void *data, const char *msg,
                                  int errnum) {
    ALogger::err("Backtrace") << msg;
}

struct BacktraceService {
    backtrace_state* backtrace;
    BacktraceService() noexcept {
        backtrace = backtrace_create_state(AProcess::self()->getPathToExecutable().toStdString().c_str(),
                                           true,
                                           aui_backtrace_error_callback,
                                           nullptr);
    }
};

static BacktraceService& backtraceService() noexcept {
    static BacktraceService s;
    return s;
}

AStacktrace AStacktrace::capture(unsigned skipFrames, unsigned maxFrames) noexcept {
    void* buffer[256];
    maxFrames += skipFrames + 1;
    assert(("too many", maxFrames <= 256));
    std::size_t entryCount = backtrace(buffer, maxFrames);

    AVector<Entry> entries;
    entries.reserve(entryCount);
    for (std::size_t i = skipFrames + 1; i < entryCount; ++i) {
        entries << Entry{buffer[i]};
    }

    return AStacktrace(std::move(entries));
}


void AStacktrace::resolveSymbolsIfNeeded() const noexcept {
    if (!mSymbolNamesResolved) {
        mSymbolNamesResolved = true;

        for (auto& entry : mEntries) {
            backtrace_pcinfo(backtraceService().backtrace,
                              reinterpret_cast<std::uintptr_t>(entry.ptr()),
                              [](void *data, uintptr_t pc,
                                 const char *filename, int lineno,
                                 const char *function) {
              auto entry = reinterpret_cast<Entry*>(data);
              if (filename) entry->mFileName = APath(filename).filename();
              if (function) {
                  int status;
                  if (auto realname = abi::__cxa_demangle(function, 0, 0, &status)) {
                      entry->mFunctionName = realname;
                      free(realname);
                  } else {
                      entry->mFunctionName = function;
                  }
              }
              entry->mLineNumber = lineno;

              return 0;
            }, aui_backtrace_error_callback, &entry);
        }
    }
}

#else
#include <AUI/Platform/AStacktrace.h>

void AStacktrace::resolveSymbolsIfNeeded() const noexcept {

}

AStacktrace AStacktrace::capture(unsigned int skipFrames, unsigned maxFrames) noexcept {
    return AStacktrace({});
}

#endif