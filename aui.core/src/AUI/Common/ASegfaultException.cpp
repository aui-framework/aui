//
// Created by alex2772 on 1/14/22.
//

#include "ASegfaultException.h"
#include "AUI/Traits/memory.h"
#include "AUI/Traits/arrays.h"

#ifdef AUI_CATCH_SEGFAULT

#if AUI_PLATFORM_WIN
// unimplemented

#else

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
static void onSegfault(int, siginfo_t * info, void *_p __attribute__ ((__unused__))) {
    printf("Caught SEGFAULT!\n");

    void* buffer[1024];
    if (int n = backtrace(buffer, aui::array_length(buffer))) {
        for (int i = 0; i < n; ++i) {
            printf(" - at %p", buffer[i]);
            Dl_info dlInfo;
            aui::zero(dlInfo);
            dladdr(buffer[i], &dlInfo);
            if (dlInfo.dli_sname) {
                int status;
                auto ptr = abi::__cxa_demangle(dlInfo.dli_sname, nullptr, nullptr, &status);
                if (!status) {
                    printf(" (%s+%p)\n", ptr, dlInfo.dli_saddr);
                } else {
                    printf(" (%s+%p)\n", dlInfo.dli_sname, dlInfo.dli_saddr);
                }
                if (ptr) free(ptr);
            } else if (dlInfo.dli_fname) {
                printf(" (%s+%p)\n", dlInfo.dli_fname, dlInfo.dli_fbase);
            } else {
                printf("\n");
            }
        }
    }


    unblock_signal(SIGSEGV);
    throw ASegfaultException(info->si_addr);
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

    }
} my_segfault_handler_registrar;

#endif
#endif