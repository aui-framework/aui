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

#include <memory>

#include "AUI/Common/AOptional.h"
#include "AUI/Common/AString.h"
#include "AUI/Common/SharedPtrTypes.h"
#include "AUI/IO/APath.h"
#include "AUI/api.h"

namespace aui::remote {
class JsonRpcDispatcher;
}

/**
 * @brief In-process agent that lets a remote client (i.e., an AI agent through `aui-remote`) inspect and drive the app.
 * @ingroup dev_extras
 * @details
 * The agent listens on the loopback interface and speaks JSON-RPC 2.0 over WebSocket. A client finds it through the
 * discovery file `~/.aui/remote/<pid>.json`, which holds the port and the token; the token is required to connect.
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
         * @brief Token clients must present. A random one is generated when empty.
         */
        AString token;
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
     * @return the token clients must present, as `?token=` of the URL or `Authorization: Bearer`
     */
    [[nodiscard]] const AString& token() const noexcept;

    /**
     * @return path to the discovery file; empty if it was not written
     */
    [[nodiscard]] const APath& discoveryFile() const noexcept;

    /**
     * @return the methods of the protocol
     */
    [[nodiscard]] aui::remote::JsonRpcDispatcher& dispatcher() noexcept;

private:
    explicit ARemoteAgent(Options options);
    struct Impl;
    std::unique_ptr<Impl> mImpl;
};
