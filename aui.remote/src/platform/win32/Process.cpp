#include "platform/Process.h"

#include <AUI/Platform/AProcess.h>

#include <memory>

#include <windows.h>
#include <tlhelp32.h>

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

std::optional<std::uint32_t> platform::launchedApplicationPid(std::uint32_t shellPid) {
    std::unique_ptr<void, decltype(&CloseHandle)> snapshot(CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0), &CloseHandle);
    if (snapshot.get() == INVALID_HANDLE_VALUE) {
        snapshot.release();
        return std::nullopt;
    }
    PROCESSENTRY32W entry { .dwSize = sizeof(entry) };
    for (bool ok = Process32FirstW(snapshot.get(), &entry); ok; ok = Process32NextW(snapshot.get(), &entry)) {
        // cmd.exe may get a console host as another child
        if (entry.th32ParentProcessID == shellPid && _wcsicmp(entry.szExeFile, L"conhost.exe") != 0) {
            return entry.th32ProcessID;
        }
    }
    return std::nullopt;
}

void platform::keepStreamsFromChildren() {
    for (auto stream : { STD_INPUT_HANDLE, STD_OUTPUT_HANDLE, STD_ERROR_HANDLE }) {
        if (HANDLE h = GetStdHandle(stream); h != nullptr && h != INVALID_HANDLE_VALUE) {
            SetHandleInformation(h, HANDLE_FLAG_INHERIT, 0);
        }
    }
}

}   // namespace aui_remote
