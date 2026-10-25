/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2025 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <atomic>
#include <functional>
#include <memory>

#include "AUI/Common/AOptional.h"
#include "AUI/Common/AString.h"
#include "AUI/Common/SharedPtrTypes.h"
#include "AUI/IO/APath.h"
#include "AUI/Json/AJson.h"
#include "AUI/Thread/AFuture.h"
#include "AUI/api.h"

namespace aui::remote {
class JsonRpcDispatcher;
class Pairing;
}

/**
 * @brief In-process agent that lets a remote client (i.e., an AI agent through `aui-remote`) inspect and drive the app.
 * @ingroup dev_extras
 * @details
 * The agent listens on the loopback interface and speaks JSON-RPC 2.0 over WebSocket. A client finds it through the
 * discovery file `~/.aui/remote/<pid>.json`, which holds the port. The first time a client connects it has to be paired:
 * the application shows a 6-digit code in its window and the client sends it back (`pair.begin`, `pair.confirm`)
 * together with a key it generated; later it connects with `?key=` and needs no code.
 *
 * It starts by itself when the application is run with the environment variable `AUI_REMOTE=1` and `aui::dev_extras`
 * is linked, no code needed. To start it from the code, call ARemoteAgent::start.
 *
 * !!! warning
 *
 *     The agent lets a client click, type and read what is on the screen. Link `aui::dev_extras` to development builds
 *     only, and do not release products alongside it.
 *
 * All methods that touch the UI are executed on the UI thread.
 */
class API_AUI_DEV_EXTRAS ARemoteAgent {
public:
    struct Options {
        /**
         * @brief Address to listen. Keep it on the loopback.
         */
        AString bindAddress = "127.0.0.1";

        /**
         * @brief Port to listen; 0 picks a free one.
         */
        std::uint16_t port = 0;

        /**
         * @brief Write `~/.aui/remote/<pid>.json` so clients can find the agent.
         */
        bool writeDiscoveryFile = true;

        /**
         * @brief A key that is trusted without pairing, i.e., when the client launches the application itself and
         * passes the key in the environment variable `AUI_REMOTE_KEY`.
         */
        AString key;
    };

    /**
     * @brief Starts the agent unless it is started already.
     * @return the running agent
     */
    static _<ARemoteAgent> start(Options options);
    static _<ARemoteAgent> start();

    /**
     * @brief Starts the agent if the environment variable AUI_REMOTE is set to 1.
     */
    static void startIfRequested();

    /**
     * @return the running agent or nullptr
     */
    static _<ARemoteAgent> running();

    ~ARemoteAgent();

    /**
     * @return the port the agent listens
     */
    [[nodiscard]] std::uint16_t port() const noexcept;

    /**
     * @brief Pairing state; lets tests and tools see the current code.
     */
    [[nodiscard]] aui::remote::Pairing& pairing() noexcept;

    /**
     * @return path to the discovery file; empty if it was not written
     */
    [[nodiscard]] const APath& discoveryFile() const noexcept;

    /**
     * @brief Exposes an application specific function to the client (`fn.list`, `fn.call`).
     * @param name name the client calls the function by
     * @param description what it does and what arguments it expects; shown by `fn.list`
     * @param handler called on the UI thread with the `arguments` of the call; the result of the future is the
     *        result of the call
     * @details
     * Useful to put the application in a state that is hard to reach through the UI, i.e., to log in or to open a
     * document.
     *
     * ```cpp
     * ARemoteAgent::start()->registerFunction("openDocument", "arguments: { path: string }", [](const AJson& args) {
     *     openDocument(args["path"].asString());
     *     AFuture<AJson> result;
     *     result.supplyValue(AJson { { "ok", true } });
     *     return result;
     * });
     * ```
     */
    void registerFunction(AString name, AString description, std::function<AFuture<AJson>(const AJson&)> handler);

    /**
     * @return the methods of the protocol
     */
    [[nodiscard]] aui::remote::JsonRpcDispatcher& dispatcher() noexcept;

private:
    explicit ARemoteAgent(Options options);
    AJson handleUnauthorized(const AString& text, std::atomic<bool>& authorized);
    struct Impl;
    std::unique_ptr<Impl> mImpl;
};
