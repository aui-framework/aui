#pragma once

#include <cstdint>
#include <optional>

namespace aui_remote::platform {

/**
 * @return whether a process with this pid exists
 */
bool isProcessAlive(std::uint32_t pid);

/**
 * @brief Politely asks the process to exit (SIGTERM on unix; there is no polite way on Windows, so it is killed).
 */
void requestTermination(std::uint32_t pid);

/**
 * @brief Kills the process that ignored requestTermination. No-op where requestTermination already kills.
 */
void forceTermination(std::uint32_t pid);

/**
 * @brief The pid of the application that `cli launch` started through the shell with this pid.
 * @details
 * On unix the shell execs the application, so the pid is the same. On Windows cmd.exe stays and the application is its
 * child; empty until the child appears.
 */
std::optional<std::uint32_t> launchedApplicationPid(std::uint32_t shellPid);

/**
 * @brief Keeps the standard streams of this process from being inherited by the processes it starts.
 * @details
 * Otherwise an application started by `cli launch` holds the stdout of aui-remote open as long as it runs, and whoever
 * reads the output of aui-remote (`$(aui-remote cli launch ...)`) waits for the application to exit. No-op on unix,
 * where the shell of `launch` replaces the streams.
 */
void keepStreamsFromChildren();

}   // namespace aui_remote::platform
