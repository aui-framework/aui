#pragma once

#include <cstdint>

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

}   // namespace aui_remote::platform
