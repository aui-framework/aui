#pragma once

#include <AUI/Common/AMap.h>
#include <AUI/Common/AOptional.h>
#include <AUI/Common/AString.h>
#include <AUI/Common/AVector.h>
#include <AUI/Json/AJson.h>

#include "remote/RpcClient.h"

namespace aui_remote::cli {

/**
 * @brief Command line of the `cli` mode split into the command, positional arguments and `--options`.
 * @details
 * An option takes the next argument as its value unless it is a known flag (see `isFlag`). Options may be repeated.
 * Everything after a lone `--` goes to `rest` untouched (used by `launch`).
 */
struct ParsedArgs {
    AString command;
    AStringVector positional;
    AMap<AString, AStringVector> options;
    AStringVector rest;

    /**
     * @return whether the option or the flag is present
     */
    [[nodiscard]] bool has(const AString& name) const { return options.contains(name); }

    /**
     * @return the last value of the option
     */
    [[nodiscard]] AOptional<AString> get(const AString& name) const;

    /**
     * @return all values of the option, in the order of appearance
     */
    [[nodiscard]] AStringVector getAll(const AString& name) const;

    [[nodiscard]] AOptional<int> getInt(const AString& name) const;

    /**
     * @throws ArgsError if the command line is malformed
     * @param args arguments after the `cli` word
     */
    static ParsedArgs parse(const AStringVector& args);

    /**
     * @return whether `--name` is a flag, i.e., it takes no value
     */
    static bool isFlag(const AString& name);
};

/**
 * @brief The command line is wrong. The message says how to fix it.
 */
class ArgsError : public AException {
public:
    using AException::AException;
};

/**
 * @brief The request of a command that is a plain call of the agent method.
 */
struct CommandRequest {
    AString method;
    AJson params;
    std::chrono::milliseconds timeout = RpcClient::DEFAULT_TIMEOUT;
};

/**
 * @brief Translates a command (`tree`, `click`, ...) into the JSON-RPC request.
 * @return nothing if the command is not an agent call (`list`, `pair`, `launch`, ...)
 * @throws ArgsError
 */
AOptional<CommandRequest> buildRequest(const ParsedArgs& args);

/**
 * @return names of all commands
 */
AStringVector commandNames();

}   // namespace aui_remote::cli
