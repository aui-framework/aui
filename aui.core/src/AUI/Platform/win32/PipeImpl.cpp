#include <AUI/Platform/Pipe.h>
#include <cassert>

Pipe::Pipe() {

    SECURITY_ATTRIBUTES securityAttributes;
    securityAttributes.nLength = sizeof(securityAttributes);
    securityAttributes.bInheritHandle = true;
    securityAttributes.lpSecurityDescriptor = nullptr;

    if (!CreatePipe(&mOut, &mIn, &securityAttributes, 0)) {
        assert((!"CreatePipe failed"));
    }
}

Pipe::~Pipe() {
    closeIn();
    closeOut();
}

void Pipe::closeIn() noexcept {
    if (mIn) {
        CloseHandle(mIn);
        mIn = nullptr;
    }
}
void Pipe::closeOut() noexcept {
    if (mOut) {
        CloseHandle(mOut);
        mOut = nullptr;
    }
}
