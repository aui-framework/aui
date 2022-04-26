#include <AUI/Platform/PipeOutputStream.h>


PipeOutputStream::PipeOutputStream(Pipe pipe) : mPipe(std::move(pipe)) {
    mFileHandle = fdopen(mPipe.in(),"w");
    assert(("invalid pipe", mFileHandle != nullptr));
}
PipeOutputStream::~PipeOutputStream() {
    fclose(mFileHandle);
}

void PipeOutputStream::write(const char* src, size_t size) {
    auto o = fwrite(src, 1, size, mFileHandle);
    assert(o == size);
}
