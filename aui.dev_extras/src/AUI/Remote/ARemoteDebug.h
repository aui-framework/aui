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
 * @brief Remote debugging: lets a client (i.e., an AI agent through `aui-remote`) inspect and drive the running app.
 * @ingroup dev_extras
 * @details
 * The agent listens on the loopback interface and speaks JSON-RPC 2.0 over secure WebSocket (`wss://`, a self-signed
 * certificate that a client pins by the fingerprint from the discovery file; there is no plain `ws://` mode). A client finds it through the
 * discovery file `~/.aui/remote/<pid>.json`, which holds the port. The first time a client connects it has to be paired:
 * the application shows a 6-digit code in its window and the client sends it back (`pair.begin`, `pair.confirm`)
 * together with a key it generated. This is done once: the application remembers the key (see
 * ARemoteDebug::Options::pairing) and the client connects with `?key=` and needs no code, even after a restart.
 *
 * It is opt-in: linking `aui::dev_extras` does nothing by itself. Call ARemoteDebug::start from your code, and guard it
 * with `AUI_DEBUG` so it never reaches release builds:
 *
 * <!-- aui:snippet examples/ui/remote_debug/src/main.cpp ARemoteDebug_include -->
 * <!-- aui:snippet examples/ui/remote_debug/src/main.cpp ARemoteDebug_start -->
 *
 * Use `__has_include` to keep the code compiling when `aui::dev_extras` is not linked. CMake side:
 *
 * <!-- aui:snippet examples/ui/remote_debug/CMakeLists.txt aui_link_dev_extras -->
 *
 * !!! warning
 *
 *     The agent lets a client click, type and read what is on the screen. Link `aui::dev_extras` to development builds
 *     only, and do not release products alongside it.
 *
 * All methods that touch the UI are executed on the UI thread.
 *
 * For a few seconds after every command, windows show a pink banner at the top and a pink pointer where the client
 * hovered or clicked (see `RemoteOverlay`). Neither is included in screenshots.
 */
class API_AUI_DEV_EXTRAS ARemoteDebug {
public:
    /**
     * @brief How long a paired client is trusted.
     */
    enum class PairingPolicy {
        /**
         * @brief A client is paired once; the application stores a hash of its key (in
         * `~/.aui/remote/trusted`) and accepts the client after restarts. The default.
         */
        REMEMBER,

        /**
         * @brief A paired client is trusted until the application exits; after a restart the code is needed again.
         * For debugging.
         */
        SESSION,

        /**
         * @brief The code is needed for every connection, even from a client with a known key. For debugging.
         */
        ALWAYS,
    };

    struct API_AUI_DEV_EXTRAS Options {
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
         * passes the key in the environment variable `AUI_REMOTE_KEY` (see ARemoteDebug::Options::fromEnvironment).
         */
        AString key;

        /**
         * @brief How long a client that has entered the code is trusted.
         */
        PairingPolicy pairing = PairingPolicy::REMEMBER;

        /**
         * @brief Options with the key taken from the environment variable `AUI_REMOTE_KEY`, which a client sets when
         * it launches the application itself. Without the variable, the key is empty and clients pair with a code.
         * The environment variable `AUI_REMOTE_PAIRING` (`remember`, `session` or `always`) sets Options::pairing.
         */
        static Options fromEnvironment();
    };

    /**
     * @brief Starts the agent unless it is started already.
     * @return the running agent
     */
    static _<ARemoteDebug> start(Options options);
    static _<ARemoteDebug> start();

    /**
     * @return the running agent or nullptr
     */
    static _<ARemoteDebug> running();

    ~ARemoteDebug();

    /**
     * @return the port the agent listens
     */
    [[nodiscard]] std::uint16_t port() const noexcept;

    /**
     * @return SHA-256 of the agent's self-signed certificate as hex; also stored in the discovery file as `certSha256`
     */
    [[nodiscard]] AString certificateFingerprint() const;

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
     * ARemoteDebug::start()->registerFunction("openDocument", "arguments: { path: string }", [](const AJson& args) {
     *     openDocument(args["path"].asString());
     *     return AFuture<AJson>::fromValue(AJson { { "ok", true } });
     * });
     * ```
     */
    void registerFunction(AString name, AString description, std::function<AFuture<AJson>(const AJson& args)> handler);

    /**
     * @return the methods of the protocol
     */
    [[nodiscard]] aui::remote::JsonRpcDispatcher& dispatcher() noexcept;

private:
    explicit ARemoteDebug(Options options);
    AJson handleUnauthorized(const AString& text, std::atomic<bool>& authorized);
    struct Impl;
    std::unique_ptr<Impl> mImpl;
};
