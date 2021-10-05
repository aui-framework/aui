/**
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
 */


#include "AException.h"
#include <AUI/Logging/ALogger.h>
class AException::Stacktrace {
public:
    struct Entry {
        std::string mName;
        int lineno;
    };

    AVector<Entry> mEntries;
};


#ifdef AUI_USE_BACKTRACE
#include <backtrace.h>
#include <AUI/Platform/AProcess.h>

void aui_backtrace_error_callback(void *data, const char *msg,
                                  int errnum) {
    ALogger::err("Backtrace error: "_as + msg);
}
int aui_backtrace_full_callback(void *data,
                                uintptr_t pc,
                                const char *filename,
                                int lineno,
                                const char *function) {
    auto exception = reinterpret_cast<AException*>(data);
    exception->mStacktrace->mEntries << AException::Stacktrace::Entry {
        function
        ? function
        : (filename
           ? filename
           : std::to_string(pc)
           ),
        lineno
    };
    return 0;
}
AException::AException() {
    mStacktrace = new AException::Stacktrace;
    static auto pathToExecutable = AProcess::self()->getPathToExecutable().toStdString();
    static auto state = backtrace_create_state(pathToExecutable.c_str(), true, aui_backtrace_error_callback, nullptr);
    backtrace_full(state, 0, aui_backtrace_full_callback, aui_backtrace_error_callback, this);
}
#else
AException::AException() {
    mStacktrace = nullptr;
}
#endif


AException::~AException() noexcept
{
    delete mStacktrace;
}

const char* AException::what() const noexcept {
    static thread_local std::string s;
    s = getMessage().toStdString();
    return s.c_str();
}

AString AException::getMessage() const {
    return mMessage;
}

void AException::printStacktrace() {
    if (!mStacktrace) {
        ALogger::warn("<stack trace is not available>");
        return;
    }
    for (auto& l : mStacktrace->mEntries) {
        ALogger::raw() << "    at " << l.mName << l.lineno << '\n';
    }
}

