#include "platform/Process.h"

#include <cerrno>
#include <signal.h>

namespace aui_remote {

bool platform::isProcessAlive(std::uint32_t pid) { return ::kill(static_cast<pid_t>(pid), 0) == 0 || errno == EPERM; }

void platform::requestTermination(std::uint32_t pid) { ::kill(static_cast<pid_t>(pid), SIGTERM); }

void platform::forceTermination(std::uint32_t pid) { ::kill(static_cast<pid_t>(pid), SIGKILL); }

}   // namespace aui_remote
