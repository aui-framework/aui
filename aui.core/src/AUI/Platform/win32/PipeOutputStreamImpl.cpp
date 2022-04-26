#include <AUI/Platform/PipeOutputStream.h>


PipeOutputStream::PipeOutputStream(Pipe pipe) : mPipe(std::move(pipe)) {}
PipeOutputStream::~PipeOutputStream() = default;

void PipeOutputStream::write(const char* src, size_t size) {
    DWORD bytesWritten;
    if (!WriteFile(mPipe.in(), src, size, &bytesWritten, nullptr)) {
        throw AIOException("failed to write to pipe");
    }
    assert(bytesWritten == size);
}
