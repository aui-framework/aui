#pragma once

#include <AUI/Common/AOptional.h>
#include <AUI/Common/AStringVector.h>
#include <AUI/IO/APath.h>

#include "Args.h"
#include "Remote/Discovery.h"

namespace aui_remote::cli {

/**
 * @brief Where the CLI looks for the applications and keeps the keys. Replaced in tests.
 */
struct Directories {
    APath discovery;
    APath keys;

    static Directories standard();
};

/**
 * @brief Picks the application a command is aimed at: `--pid N` or the only running one.
 * @throws RpcError listing the candidates in `data.candidates` if there is no unambiguous choice
 */
AppInfo selectApp(const ParsedArgs& args, const AVector<AppInfo>& apps);

/**
 * @brief Runs the CLI if the first argument is `cli`.
 * @param args the whole command line including the program name
 * @return the exit code; nothing if the GUI should start
 */
AOptional<int> tryRun(const AStringVector& args);

/**
 * @brief Runs a CLI command.
 * @param args the command line after the `cli` word
 * @details
 * The result (or the error) is printed to stdout as one JSON object on the last line; messages for humans go to stderr.
 * @return the exit code
 */
int run(const AStringVector& args, const Directories& directories = Directories::standard());

}   // namespace aui_remote::cli
