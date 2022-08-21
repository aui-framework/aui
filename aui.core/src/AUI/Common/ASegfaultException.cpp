//
// Created by alex2772 on 1/14/22.
//

#include "ASegfaultException.h"
#include "AUI/Traits/memory.h"


#ifdef AUI_CATCH_SEGFAULT

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
static void unblock_signal(int signum __attribute__((__unused__)))
{
#ifdef _POSIX_VERSION
    sigset_t sigs;
    sigemptyset(&sigs);
    sigaddset(&sigs, signum);
    sigprocmask(SIG_UNBLOCK, &sigs, NULL);
#endif
}
static void onSegfault(int c, siginfo_t * info, void *_p __attribute__ ((__unused__))) {
    switch (c) {
    case SIGSEGV:
        std::cout << "Caught SEGFAULT!";
        break;

    case SIGABRT:
        std::cout << "Caught assertion fail!";
        break;
    }
    std::cout << std::endl << AStacktrace::capture(3);

    if (c == SIGSEGV) {
        unblock_signal(SIGSEGV);
        throw ASegfaultException(info->si_addr);
    }
}

struct segfault_handler_registrar {
    segfault_handler_registrar() noexcept {
        struct sigaction act;
        aui::zero(act);
        act.sa_sigaction = onSegfault;
        sigemptyset (&act.sa_mask);
        act.sa_flags = SA_SIGINFO|0x4000000;
        act.sa_restorer = restoreRt;
        //auto r = syscall(SYS_rt_sigaction, SIGSEGV, &act, nullptr, _NSIG / 8);
        //assert(r == 0);
        sigaction(SIGSEGV, &act, nullptr);
        sigaction(SIGABRT, &act, nullptr); // for assertions

    }
} my_segfault_handler_registrar;

#endif
#endif