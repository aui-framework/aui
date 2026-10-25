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

#include <chrono>
#include <functional>
#include <memory>

#include <AUI/Common/AByteBuffer.h>
#include <AUI/Common/AException.h>
#include <AUI/Common/AOptional.h>
#include <AUI/Common/SharedPtrTypes.h>
#include <AUI/api.h>

#include "ATlsIdentity.h"

/**
 * @brief TLS failure: bad handshake, fingerprint mismatch, broken or corrupted stream.
 * @ingroup crypt
 */
class API_AUI_CRYPT ATlsException : public AException {
public:
    using AException::AException;
};

/**
 * @brief Bytes pipe a TLS session runs on, i.e., a TCP socket.
 * @ingroup crypt
 */
struct ATlsTransport {
    /**
     * @brief Waits until `read` would not block. True if there is something to read or the peer is gone.
     */
    std::function<bool(std::chrono::milliseconds)> waitForRead;

    /**
     * @brief Reads at most `size` bytes, blocks until there is something. Returns 0 on end of stream.
     */
    std::function<std::size_t(char* dst, std::size_t size)> read;

    /**
     * @brief Writes all `size` bytes.
     */
    std::function<void(const char* src, std::size_t size)> write;
};

/**
 * @brief TLS (1.2 / 1.3) over an arbitrary transport.
 * @ingroup crypt
 * @details
 * A thin wrapper around mbedTLS that keeps it out of the public headers.
 *
 * Server:
 *
 * ```cpp
 * auto session = ATlsSession::accept(transport, identity);
 * auto n = session->read(buffer, sizeof(buffer));
 * session->write("hello", 5);
 * ```
 *
 * Client:
 *
 * ```cpp
 * ATlsSession::ClientOptions options;
 * options.expectedFingerprint = fingerprintFromTrustedChannel;
 * auto session = ATlsSession::connect(transport, options);
 * ```
 *
 * ### Threading
 *
 * One thread may `read` while another `write`s; the calls are serialized internally and a `read` waiting for data does
 * not hold up `write`.
 *
 * !!! note
 *
 *     Closing the underlying socket from another thread is the way to interrupt a `read`.
 */
class API_AUI_CRYPT ATlsSession {
public:
    struct ClientOptions {
        /**
         * @brief SHA-256 of the server certificate that is accepted. The handshake fails with ATlsException on any other
         * certificate. There is no other way to authenticate the server, as no certificate chain is verified.
         */
        AOptional<AByteBuffer> expectedFingerprint;

        /**
         * @brief Server name for SNI; optional.
         */
        AString serverName;
    };

    ~ATlsSession();
    ATlsSession(const ATlsSession&) = delete;

    /**
     * @brief Performs the server side of the handshake.
     * @throws ATlsException on failure or if the handshake does not finish in `timeout`
     */
    [[nodiscard]] static _<ATlsSession>
    accept(ATlsTransport transport, _<ATlsIdentity> identity,
           std::chrono::milliseconds timeout = std::chrono::seconds(10));

    /**
     * @brief Performs the client side of the handshake.
     * @throws ATlsException on failure, certificate mismatch or if the handshake does not finish in `timeout`
     */
    [[nodiscard]] static _<ATlsSession>
    connect(ATlsTransport transport, const ClientOptions& options = {},
            std::chrono::milliseconds timeout = std::chrono::seconds(10));

    /**
     * @brief Reads decrypted data; blocks until there is some.
     * @return number of bytes; 0 if the peer closed the session
     * @throws ATlsException when the stream is broken; the transport's exception is rethrown as is
     */
    std::size_t read(char* dst, std::size_t size);

    /**
     * @brief Encrypts and writes all the data.
     */
    void write(const char* src, std::size_t size);

    /**
     * @brief Sends close_notify. Does not close the transport.
     */
    void close() noexcept;

    /**
     * @return SHA-256 of the certificate the peer presented
     */
    [[nodiscard]] AOptional<AByteBuffer> peerFingerprintSha256() const;

private:
    struct Impl;
    explicit ATlsSession(std::unique_ptr<Impl> impl);
    std::unique_ptr<Impl> mImpl;
};
