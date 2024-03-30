#include <AUI/Platform/AProcess.h>
#import <Foundation/Foundation.h>
#include <mach-o/dyld.h>
#include <unistd.h>
#include "AUI/Common/AException.h"

class ASelfProcess : public AProcess {
   public:
    ASelfProcess() {}

    ~ASelfProcess() {}

    int waitForExitCode() override {
        int loc;
        waitpid(getpid(), &loc, 0);
        return WEXITSTATUS(loc);
    }

    APath getModuleName() override { return getPathToExecutable().filename(); }

    APath getPathToExecutable() override {
        char buf[0x800];
        uint32_t size = sizeof(buf);
        if (_NSGetExecutablePath(buf, &size)) {
            throw AException("_NSGetExecutablePath failed");
        }
        return buf;
    }

    uint32_t getPid() const noexcept override { return getpid(); }

    size_t processMemory() const override { throw AException("unimplemented"); }
};

_<AProcess> AProcess::self() { return _new<ASelfProcess>(); }

_<AProcess> AProcess::fromPid(uint32_t pid) { throw AException("unimplemented"); }