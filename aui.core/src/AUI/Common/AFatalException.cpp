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
// Created by alex2772 on 1/14/22.
//

#include "AFatalException.h"
#include "AUI/Traits/memory.h"
#include "AUI/Logging/ALogger.h"


AFatalException::Handler& AFatalException::handler() {
    static Handler h;
    return h;
}

AString AFatalException::getMessage() const noexcept {
    return "{} at address {}"_format(mSignalName, mAddress);
}

#ifdef AUI_CATCH_SEGFAULT

extern "C" {
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
}

static void __cdecl onSignal(int c) {
    const char* signalName = "unknown signal";

    switch (c) {
        case SIGILL:
            signalName = "Illegal instruction";
            break;
        case SIGFPE:
            signalName = "floating point exception";
            break;
        case SIGSEGV:
            signalName = "access violation";
            break;
        case SIGABRT:
            signalName = "abort";
            break;
    }

    ALogger::err("SignalHandler") << "Caught signal: " << signalName << "(" << c << ")\n" << AStacktrace::capture(3);
    void* addr = 0;
    auto e = AStacktrace::capture(c == SIGABRT ? 5 : 7, 1).entries();
    if (!e.empty()) {
        addr = e.first().ptr();
    }

    throw AFatalException(addr, signalName);
}

struct segfault_handler_registrar {
    segfault_handler_registrar() noexcept {
        signal(SIGILL, onSignal);
        signal(SIGFPE, onSignal);
        signal(SIGSEGV, onSignal);
        signal(SIGABRT, onSignal); // for assertions

    }
} my_segfault_handler_registrar;

#if AUI_PLATFORM_WIN
// unimplemented

#elif AUI_PLATFORM_LINUX

extern "C" {
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <execinfo.h>
#include <dlfcn.h>
}

#include <cxxabi.h>
void restoreRt() {
    printf("restoreRt");
}
static void unblockSignal(int signum __attribute__((__unused__)))
{
#ifdef _POSIX_VERSION
    sigset_t sigs;
    sigemptyset(&sigs);
    sigaddset(&sigs, signum);
    sigprocmask(SIG_UNBLOCK, &sigs, NULL);
#endif
}
static void onSignal(int c, siginfo_t * info, void *_p __attribute__ ((__unused__))) {
    const char* signalName = strsignal(c);
    if (!signalName) signalName = "unknown signal";

    ALogger::err("SignalHandler") << "Caught signal: " << signalName << "(" << c << ")\n" << AStacktrace::capture(3);

    unblockSignal(c);
    throw AFatalException(info->si_addr, signalName);
}

struct segfault_handler_registrar {
    segfault_handler_registrar() noexcept {
        struct sigaction act;
        aui::zero(act);
        act.sa_sigaction = onSignal;
        sigemptyset (&act.sa_mask);
        act.sa_flags = SA_SIGINFO|0x4000000;
        act.sa_restorer = restoreRt;
        //auto r = syscall(SYS_rt_sigaction, SIGSEGV, &act, nullptr, _NSIG / 8);
        //assert(r == 0);
        sigaction(SIGILL, &act, nullptr);
        sigaction(SIGFPE, &act, nullptr);
        sigaction(SIGSEGV, &act, nullptr);
        sigaction(SIGABRT, &act, nullptr); // for assertions

    }
} my_segfault_handler_registrar;

#endif
#endif
