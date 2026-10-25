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

#include <cstdint>
#include <functional>
#include <memory>

#include "AUI/ACurl.h"
#include "AUI/Common/AByteBuffer.h"
#include "AUI/Common/AByteBufferView.h"
#include "AUI/Common/AMap.h"
#include "AUI/Common/AOptional.h"
#include "AUI/Common/AString.h"
#include "AUI/Common/SharedPtrTypes.h"
#include "AUI/Crypt/ATlsIdentity.h"

/**
 * @brief WebSocket server (RFC 6455) listening on a TCP port.
 * @ingroup curl
 * @details
 * The counterpart of [AWebsocket]. Despite living in aui::curl, it does not use libcurl: the handshake and framing are
 * implemented on top of [ATcpServerSocket].
 *
 * ```cpp
 * AWebsocketServer::Config config;
 * config.onConnected = [](_<AWebsocketServer::Connection> connection) {
 *     connection->onMessage = [connection](AByteBuffer message, bool binary) {
 *         connection->sendText(AString::fromUtf8(message)); // echo
 *     };
 * };
 * AWebsocketServer server(std::move(config)); // loopback, random port
 * server.start();
 * ALogger::info("ws") << "listening on port " << server.port();
 * ```
 *
 * ### Threading
 *
 * One thread accepts connections and one thread per connection reads from the socket. Every callback is called from
 * one of these threads, never from the thread that created the server; marshal to your thread (i.e., `AUI_UI_THREAD`)
 * if needed. `Connection::send*` may be called from any thread.
 *
 * ### Encryption
 *
 * Set `Config::tls` to serve `wss://`. A client that fails the handshake, i.e., one that speaks plain `ws://`, is
 * dropped without any response.
 *
 * ### Limits
 *
 * The handshake must complete within `handshakeTimeout`; the request headers are limited by `maxHeaderSize` and a
 * message (assembled from fragments) is limited by `maxMessageSize`, otherwise the connection is closed with
 * code 1009. Text messages are delivered as is, without UTF-8 validation.
 *
 * !!! warning
 *
 *     By default the server listens on the loopback interface only. Any process of the machine (and any web page, in
 *     the browser, unless the `Origin` header is checked) can connect to it, so use `verifyHandshake` to authenticate
 *     clients if anything sensitive is exposed.
 */
class API_AUI_CURL AWebsocketServer {
public:
    /**
     * @brief Parsed HTTP upgrade request.
     */
    struct Request {
        AString method;

        /**
         * @brief Request target as sent by the client, including the query string.
         */
        AString target;

        /**
         * @brief Header values by lower-cased header name.
         */
        AMap<AString, AString> headers;

        /**
         * @brief Query string parameter (`/path?name=value`). Not percent-decoded.
         */
        [[nodiscard]] AOptional<AString> queryParameter(const AString& name) const;
    };

    class Connection;

    struct Config {
        /**
         * @brief Address to listen. Port 0 picks a free port, see `AWebsocketServer::port()`.
         */
        AString bindAddress = "127.0.0.1";
        std::uint16_t port = 0;

        /**
         * @brief Makes the server speak `wss://`: every connection starts with a TLS handshake using this identity.
         * @details
         * Without it the server speaks plain `ws://`, so everything, including secrets, is visible to whoever can
         * sniff the traffic. A self-signed identity ([ATlsIdentity::generateSelfSigned]) is fine when the client gets
         * its fingerprint from a trusted channel.
         */
        _<ATlsIdentity> tls;

        std::size_t maxHeaderSize = 16 * 1024;
        std::size_t maxMessageSize = 64 * 1024 * 1024;
        std::chrono::milliseconds handshakeTimeout = std::chrono::seconds(10);

        /**
         * @brief Called for every well-formed upgrade request. Return false to answer with 403.
         */
        std::function<bool(const Request&)> verifyHandshake;

        /**
         * @brief Called after the handshake. Set the connection's callbacks here.
         */
        std::function<void(_<Connection>)> onConnected;
    };

    class API_AUI_CURL Connection {
    public:
        virtual ~Connection() = default;

        /**
         * @brief Called for each complete message. `binary` is true for binary messages.
         */
        std::function<void(AByteBuffer message, bool binary)> onMessage;

        /**
         * @brief Called once, when the connection is over (closed by either side or broken).
         */
        std::function<void()> onClosed;

        [[nodiscard]] virtual const Request& request() const = 0;

        /**
         * @return false if the connection is already closed.
         */
        virtual bool sendText(AStringView text) = 0;
        virtual bool sendBinary(AByteBufferView data) = 0;

        /**
         * @brief Sends CLOSE frame and drops the connection.
         */
        virtual void close(std::uint16_t code = 1000) = 0;

        [[nodiscard]] virtual bool isOpen() const = 0;
    };

    explicit AWebsocketServer(Config config);
    ~AWebsocketServer();
    AWebsocketServer(const AWebsocketServer&) = delete;

    /**
     * @brief Starts accepting connections in a background thread.
     */
    void start();

    /**
     * @brief Closes all connections and waits for the threads. Called by the destructor.
     */
    void stop();

    /**
     * @return the port the server listens, which is the picked one if `Config::port` is 0.
     */
    [[nodiscard]] std::uint16_t port() const noexcept;

private:
    struct Impl;
    std::unique_ptr<Impl> mImpl;
};
