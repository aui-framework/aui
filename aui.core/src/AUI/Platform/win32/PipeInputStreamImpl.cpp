#include <AUI/Platform/PipeInputStream.h>

PipeInputStream::PipeInputStream(Pipe pipe) : mPipe(std::move(pipe)) {}
PipeInputStream::~PipeInputStream() = default;

size_t PipeInputStream::read(char* dst, size_t size) {
    DWORD bytesRead;
    if (!ReadFile(mPipe.out(), dst, size, &bytesRead, nullptr)) {
        if (GetLastError() == ERROR_BROKEN_PIPE) {
            return 0;
        }
        throw AIOException("failed to read from pipe");
    }
    return bytesRead;
}