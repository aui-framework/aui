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

//
// Created by alex2772 on 9/5/22.
//

#pragma once

#include <atomic>
#include <chrono>
#include <mutex>

#include "AUI/Common/AByteBuffer.h"
#include "AUI/Common/AObject.h"
#include "AUI/Common/AOptional.h"
#include "AUI/Common/ASignal.h"
#include "AUI/Common/AString.h"
#include "AUI/Curl/Detail/WebsocketFrame.h"
#include "AUI/IO/IOutputStream.h"

class ATcpSocket;
class ATlsSession;

/**
 * @brief The server did not accept the WebSocket upgrade.
 * @ingroup curl
 */
class API_AUI_CURL AWebsocketHandshakeException : public AException {
public:
    using AException::AException;
};

/**
 * @brief WebSocket client (ws:// and wss://).
 * @ingroup curl
 * @details
 * `run()` connects and blocks until the connection is over. Connection, incoming messages and closing are reported with
 * signals, emitted on the thread that called `run()`. Outgoing messages are sent with `writeText` and `writeBinary` (or
 * by using AWebsocket as an `IOutputStream`, which sends text frames) from any thread.
 *
 * ```cpp
 * auto ws = _new<AWebsocket>("wss://ws.postman-echo.com/raw");
 * AObject::connect(ws->connected, ws, [&] { ws->writeText("hello"); });
 * AObject::connect(ws->received, ws, [&](AByteBufferView data) {
 *     ALogger::info("ws") << AString::fromUtf8(data);
 *     ws->close();
 * });
 * ws->run(); // blocks until closed
 * ```
 *
 * Instead of `run()` and signals, a thread can call `connect()` and pull the messages with `read`:
 *
 * ```cpp
 * AWebsocket ws("wss://127.0.0.1:1234/", { .expectedFingerprint = fingerprint });
 * ws.connect();
 * ws.writeText("hello");
 * if (auto message = ws.read(std::chrono::seconds(5))) { ... }
 * ```
 *
 * By default, wss:// encrypts the traffic but does not authenticate the server: no certificate chain is verified. Pass
 * `Options::expectedFingerprint` to accept a single known certificate.
 */
class API_AUI_CURL AWebsocket : public AObject, public IOutputStream {
    friend class WebsocketTest_Receive1_Test;
    friend class WebsocketTest_Receive2_Test;
    friend class WebsocketTest_FragmentedMessageIsAssembled_Test;

public:
    struct Options {
        /**
         * @brief SHA-256 of the server certificate that is accepted (wss:// only). The handshake fails on any other.
         */
        AOptional<AByteBuffer> expectedFingerprint;

        /**
         * @brief Limit of the TLS handshake and of the HTTP upgrade each.
         */
        std::chrono::milliseconds connectTimeout = std::chrono::seconds(10);

        /**
         * @brief Upper bound of an incoming message.
         */
        std::size_t maxMessageSize = 1 << 28;
    };

    explicit AWebsocket(AString url);
    AWebsocket(AString url, Options options);
    ~AWebsocket() override;

    /**
     * @brief Connects and performs the handshake.
     * @throws AWebsocketHandshakeException if the server refuses the connection
     * @throws AException (AIOException, ATlsException...) on a transport failure
     */
    void connect();

    /**
     * @brief Connects and emits signals until the connection is closed by either side.
     */
    void run();

    /**
     * @return the next message; nothing if there is none in `timeout`
     * @throws AIOException when the connection is closed
     * @throws aui::websocket::ProtocolError if the server breaks the protocol
     */
    [[nodiscard]] AOptional<AByteBuffer> read(std::chrono::milliseconds timeout);

    void write(const char* src, size_t size) override { writeText(AStringView(src, size)); }
    void writeText(AStringView text);
    void writeBinary(AByteBufferView data);

    /**
     * @brief Tells the server goodbye and closes the connection. Interrupts `run()` and `read`. Can be called from any
     * thread.
     */
    void close();

private:
    AWebsocket();   // for tests

    AString mUrl;
    Options mOptions;
    _<ATcpSocket> mSocket;
    _<ATlsSession> mTls;
    aui::websocket::FrameDecoder mDecoder;
    AByteBuffer mAssembling;
    bool mAssemblingActive = false;
    bool mPeerGone = false;
    std::atomic_bool mCloseRequested = false;
    std::mutex mSendMutex;

    void send(aui::websocket::Opcode opcode, AByteBufferView payload);
    std::size_t readSome(char* dst, std::size_t size);
    void handshake(const AString& host, const AString& target);

    /// Takes the bytes that have arrived in `timeout`, if any, to the decoder.
    void pull(std::chrono::milliseconds timeout);

    /// Handles control frames on the way.
    [[nodiscard]] AOptional<AByteBuffer> nextMessage();

    std::size_t onDataReceived(AByteBufferView data);

signals:
    emits<> connected;
    emits<AByteBuffer> received;
    emits<AString /* message */> websocketClosed;
};
