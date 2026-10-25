#pragma once

#include <chrono>
#include <cstdint>
#include <functional>

#include <AUI/Common/AString.h>
#include <AUI/Common/AVector.h>
#include <AUI/IO/APath.h>
#include <AUI/Json/AJson.h>

namespace aui_remote {

/**
 * @brief An application with a running remote agent, as described by its discovery file.
 */
struct AppInfo {
    int protocol = 1;
    std::uint32_t pid = 0;
    std::uint16_t port = 0;
    AString exe;
    AStringVector argv;
    AString cwd;

    /**
     * @brief Start time as written by the agent. Together with the pid it identifies the process.
     */
    AString startedAt;

    /**
     * @return exe file name and the pid, i.e., "myapp (1234)"
     */
    [[nodiscard]] AString displayName() const;

    bool operator==(const AppInfo&) const = default;

    /**
     * @brief Parses the content of a discovery file.
     * @throws AException if the content is not a valid discovery file
     */
    static AppInfo fromJson(const AJson& json);
};

namespace discovery {

/**
 * @return `~/.aui/remote`
 */
APath defaultDirectory();

/**
 * @return whether a process with this pid exists
 */
bool isProcessAlive(std::uint32_t pid);

/**
 * @brief Asks the process to exit; kills it if it is still alive after `grace`.
 * @return whether the process is gone
 */
bool terminateProcess(std::uint32_t pid, std::chrono::milliseconds grace = std::chrono::seconds(3));

/**
 * @brief Lists the applications described in `directory`.
 * @param isAlive tells whether a pid is alive; the discovery files of dead processes are deleted
 * @details
 * Files that are not valid discovery files are skipped, but not deleted. The result is sorted by pid.
 */
AVector<AppInfo> list(const APath& directory = defaultDirectory(),
                      const std::function<bool(std::uint32_t)>& isAlive = isProcessAlive);

/**
 * @brief Reads `<pid>.json` from `directory`.
 * @return nothing if there is no such file
 */
AOptional<AppInfo> find(std::uint32_t pid, const APath& directory = defaultDirectory());

}   // namespace discovery
}   // namespace aui_remote
