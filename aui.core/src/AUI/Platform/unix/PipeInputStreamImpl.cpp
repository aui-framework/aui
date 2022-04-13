#include <AUI/Platform/PipeInputStream.h>

PipeInputStream::PipeInputStream(Pipe pipe) : mPipe(std::move(pipe)) {
    mFileHandle = fdopen(mPipe.out(),"r");
    assert(("invalid pipe", mFileHandle != nullptr));
}

PipeInputStream::~PipeInputStream() {
    fclose(mFileHandle);
}

size_t PipeInputStream::read(char* dst, size_t size) {
    return fread(dst, 1, size, mFileHandle);
}

