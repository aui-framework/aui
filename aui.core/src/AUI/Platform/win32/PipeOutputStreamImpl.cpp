#include <AUI/Platform/PipeOutputStream.h>


void PipeOutputStream::write(const char* src, size_t size) {
    DWORD bytesWritten;
    if (!WriteFile(mPipe.in(), src, size, &bytesWritten, nullptr)) {
        throw AIOException("failed to write to pipe");
    }
    assert(bytesWritten == size);
}