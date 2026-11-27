#pragma once

#include "Args.h"
#include "Cli.h"

namespace aui_remote::cli {

/**
 * @brief Whether the agent of an attached application answers (a connection without a key is enough).
 */
bool isAgentReachable(const AppInfo& app);

/**
 * @brief `attach`: makes an application on another machine visible to the other commands.
 * @details
 * - `attach --adb [--serial S] [--lan]`: an Android device. The agent writes its discovery file on the device, where
 *   nobody can read it, so it also prints the same JSON to logcat; it is taken from there. Without `--lan` the port is
 *   `adb forward`ed to this machine; with `--lan` the client connects to the address of the device in the local network
 *   (the application has to listen there, `AUI_REMOTE_BIND` / `debug.aui.remote.bind`).
 * - `attach --host H --port P --cert SHA256 [--pid N] [--name NAME]`: anything else that can be reached over the
 *   network, whose fingerprint is known.
 * @return the entry that was stored
 */
AJson doAttach(const ParsedArgs& args, const Directories& directories);

/**
 * @brief `detach <pid>`: forgets an attached application (and removes the `adb forward` of it).
 */
AJson doDetach(const ParsedArgs& args, const Directories& directories);

}   // namespace aui_remote::cli
