#include <AUI/Platform/Pipe.h>
#include <cassert>
#include "AUI/Common/AException.h"

Pipe::Pipe() {

    if (pipe(&mOut) == -1) {
        throw AException("could not create unix pipe");
    }
}

Pipe::~Pipe() {
    closeIn();
    closeOut();
}

void Pipe::closeIn() noexcept {
    if (mIn) {
        close(mIn);
        mIn = 0;
    }
}
void Pipe::closeOut() noexcept {
    if (mOut) {
        close(mOut);
        mOut = 0;
    }
}
