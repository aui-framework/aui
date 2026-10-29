#include "platform/Process.h"

#include <AUI/Platform/AProcess.h>

#include <windows.h>

namespace aui_remote {

bool platform::isProcessAlive(std::uint32_t pid) {
    HANDLE h = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (h == nullptr) {
        return GetLastError() == ERROR_ACCESS_DENIED;
    }
    DWORD code = 0;
    const bool alive = GetExitCodeProcess(h, &code) && code == STILL_ACTIVE;
    CloseHandle(h);
    return alive;
}

void platform::requestTermination(std::uint32_t pid) {
    if (auto process = AProcess::fromPid(pid)) {
        process->kill();
    }
}

void platform::forceTermination(std::uint32_t) {}

}   // namespace aui_remote
