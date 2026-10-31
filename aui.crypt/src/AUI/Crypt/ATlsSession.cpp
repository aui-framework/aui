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

#include "ATlsSession.h"

#include <algorithm>
#include <exception>
#include <mutex>

#include <mbedtls/ctr_drbg.h>
#include <mbedtls/entropy.h>
#include <mbedtls/error.h>
#include <mbedtls/net_sockets.h>
#include <mbedtls/psa_util.h>
#include <mbedtls/ssl.h>
#include <psa/crypto.h>

#include "AHash.h"
#include "TlsInternal.h"

namespace aui::tls {
void init() {
    static const psa_status_t status = psa_crypto_init();
    if (status != PSA_SUCCESS) {
        throw ATlsException("unable to initialize the crypto subsystem");
    }
}

int random(void*, unsigned char* output, std::size_t size) {
    struct Generator {
        std::mutex mutex;
        mbedtls_entropy_context entropy;
        mbedtls_ctr_drbg_context drbg;
        Generator() {
            mbedtls_entropy_init(&entropy);
            mbedtls_ctr_drbg_init(&drbg);
            const char personalization[] = "aui.crypt.tls";
            if (mbedtls_ctr_drbg_seed(&drbg, mbedtls_entropy_func, &entropy,
                                      reinterpret_cast<const unsigned char*>(personalization),
                                      sizeof(personalization) - 1) != 0) {
                throw ATlsException("unable to seed the random generator");
            }
        }
    };
    static Generator generator;
    std::lock_guard lock(generator.mutex);
    while (size > 0) {
        const auto chunk = std::min<std::size_t>(size, MBEDTLS_CTR_DRBG_MAX_REQUEST);
        if (const int r = mbedtls_ctr_drbg_random(&generator.drbg, output, chunk)) {
            return r;
        }
        output += chunk;
        size -= chunk;
    }
    return 0;
}

AString describe(int code) {
    char buffer[200];
    mbedtls_strerror(code, buffer, sizeof(buffer));
    return buffer;
}
}   // namespace aui::tls

using namespace std::chrono_literals;

struct ATlsSession::Impl {
    ATlsTransport transport;
    _<ATlsIdentity> identity;   // keeps the certificate alive
    mbedtls_ssl_context ssl;
    mbedtls_ssl_config config;
    std::mutex mutex;
    std::exception_ptr transportError;   // an exception thrown by the transport inside of an mbedTLS callback
    AOptional<AByteBuffer> peerFingerprint;
    bool closed = false;

    Impl() {
        mbedtls_ssl_init(&ssl);
        mbedtls_ssl_config_init(&config);
    }

    ~Impl() {
        mbedtls_ssl_free(&ssl);
        mbedtls_ssl_config_free(&config);
    }

    static int send(void* ctx, const unsigned char* buffer, std::size_t size) {
        auto* self = static_cast<Impl*>(ctx);
        try {
            self->transport.write(std::as_bytes(std::span(buffer, size)));
            return static_cast<int>(size);
        } catch (...) {
            self->transportError = std::current_exception();
            return MBEDTLS_ERR_NET_SEND_FAILED;
        }
    }

    static int receive(void* ctx, unsigned char* buffer, std::size_t size) {
        auto* self = static_cast<Impl*>(ctx);
        try {
            // never block under the lock: the thread that waits for the data does it outside, see waitForRead
            if (!self->transport.waitForRead(0ms)) {
                return MBEDTLS_ERR_SSL_WANT_READ;
            }
            const auto n = self->transport.read(std::as_writable_bytes(std::span(buffer, size)));
            return static_cast<int>(n);   // 0 is end of stream
        } catch (...) {
            self->transportError = std::current_exception();
            return MBEDTLS_ERR_NET_RECV_FAILED;
        }
    }

    [[noreturn]] void fail(int code, const char* what) {
        if (transportError) {
            std::rethrow_exception(std::exchange(transportError, nullptr));
        }
        throw ATlsException(AString(what) + ": " + aui::tls::describe(code));
    }

    /// Waits for the transport without holding the lock.
    void waitForData(std::chrono::milliseconds limit) { transport.waitForRead(limit); }

    void handshake(std::chrono::milliseconds timeout) {
        const auto deadline = std::chrono::steady_clock::now() + timeout;
        for (;;) {
            int result;
            {
                std::lock_guard lock(mutex);
                result = mbedtls_ssl_handshake(&ssl);
                if (result != 0 && result != MBEDTLS_ERR_SSL_WANT_READ && result != MBEDTLS_ERR_SSL_WANT_WRITE) {
                    fail(result, "TLS handshake failed");
                }
            }
            if (result == 0) {
                return;
            }
            const auto left = std::chrono::duration_cast<std::chrono::milliseconds>(
                deadline - std::chrono::steady_clock::now());
            if (left.count() <= 0) {
                throw ATlsException("TLS handshake timed out");
            }
            if (result == MBEDTLS_ERR_SSL_WANT_READ) {
                waitForData(std::min<std::chrono::milliseconds>(left, 50ms));
            }
        }
    }

    void rememberPeer() {
        if (const auto* cert = mbedtls_ssl_get_peer_cert(&ssl)) {
            peerFingerprint = AHash::sha256(AByteBufferView(reinterpret_cast<const char*>(cert->raw.p), cert->raw.len));
        }
    }
};

ATlsSession::ATlsSession(std::unique_ptr<Impl> impl) : mImpl(std::move(impl)) {}

ATlsSession::~ATlsSession() = default;

_<ATlsSession> ATlsSession::accept(ATlsTransport transport, _<ATlsIdentity> identity,
                                   std::chrono::milliseconds timeout) {
    aui::tls::init();
    auto impl = std::make_unique<Impl>();
    impl->transport = std::move(transport);
    impl->identity = std::move(identity);

    auto fail = [&](int code, const char* what) { throw ATlsException(AString(what) + ": " + aui::tls::describe(code)); };
    if (int r = mbedtls_ssl_config_defaults(&impl->config, MBEDTLS_SSL_IS_SERVER, MBEDTLS_SSL_TRANSPORT_STREAM,
                                            MBEDTLS_SSL_PRESET_DEFAULT)) {
        fail(r, "unable to configure TLS");
    }
    mbedtls_ssl_conf_rng(&impl->config, aui::tls::random, nullptr);
    mbedtls_ssl_conf_min_tls_version(&impl->config, MBEDTLS_SSL_VERSION_TLS1_2);
    mbedtls_ssl_conf_authmode(&impl->config, MBEDTLS_SSL_VERIFY_NONE);
    if (int r = mbedtls_ssl_conf_own_cert(
            &impl->config, &impl->identity->mImpl->certificate,
            &impl->identity->mImpl->key)) {
        fail(r, "unable to set the certificate");
    }
    if (int r = mbedtls_ssl_setup(&impl->ssl, &impl->config)) {
        fail(r, "unable to set up TLS");
    }
    mbedtls_ssl_set_bio(&impl->ssl, impl.get(), Impl::send, Impl::receive, nullptr);
    impl->handshake(timeout);
    return aui::ptr::manage_shared(new ATlsSession(std::move(impl)));
}

_<ATlsSession> ATlsSession::connect(ATlsTransport transport, const ClientOptions& options,
                                    std::chrono::milliseconds timeout) {
    aui::tls::init();
    auto impl = std::make_unique<Impl>();
    impl->transport = std::move(transport);

    auto fail = [&](int code, const char* what) { throw ATlsException(AString(what) + ": " + aui::tls::describe(code)); };
    if (int r = mbedtls_ssl_config_defaults(&impl->config, MBEDTLS_SSL_IS_CLIENT, MBEDTLS_SSL_TRANSPORT_STREAM,
                                            MBEDTLS_SSL_PRESET_DEFAULT)) {
        fail(r, "unable to configure TLS");
    }
    mbedtls_ssl_conf_rng(&impl->config, aui::tls::random, nullptr);
    mbedtls_ssl_conf_min_tls_version(&impl->config, MBEDTLS_SSL_VERSION_TLS1_2);
    // no chain to verify; the certificate is checked against the fingerprint below
    mbedtls_ssl_conf_authmode(&impl->config, MBEDTLS_SSL_VERIFY_NONE);
    if (int r = mbedtls_ssl_setup(&impl->ssl, &impl->config)) {
        fail(r, "unable to set up TLS");
    }
    if (!options.serverName.empty()) {
        if (int r = mbedtls_ssl_set_hostname(&impl->ssl, options.serverName.toStdString().c_str())) {
            fail(r, "unable to set the server name");
        }
    }
    mbedtls_ssl_set_bio(&impl->ssl, impl.get(), Impl::send, Impl::receive, nullptr);
    impl->handshake(timeout);
    impl->rememberPeer();
    if (options.expectedFingerprint) {
        const auto& expected = *options.expectedFingerprint;
        const bool match = impl->peerFingerprint && impl->peerFingerprint->size() == expected.size() &&
                           std::equal(expected.data(), expected.data() + expected.size(),
                                      impl->peerFingerprint->data());
        if (!match) {
            throw ATlsException("the server certificate does not match the expected fingerprint");
        }
    }
    return aui::ptr::manage_shared(new ATlsSession(std::move(impl)));
}

std::size_t ATlsSession::read(std::span<std::byte> destination) {
    auto& impl = *mImpl;
    for (;;) {
        int result;
        {
            std::lock_guard lock(impl.mutex);
            if (impl.closed) {
                return 0;
            }
            result = mbedtls_ssl_read(&impl.ssl, reinterpret_cast<unsigned char*>(destination.data()), destination.size());
            if (result > 0) {
                return static_cast<std::size_t>(result);
            }
            if (result == 0 || result == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY || result == MBEDTLS_ERR_NET_CONN_RESET) {
                return 0;
            }
            if (result != MBEDTLS_ERR_SSL_WANT_READ && result != MBEDTLS_ERR_SSL_WANT_WRITE &&
                result != MBEDTLS_ERR_SSL_RECEIVED_NEW_SESSION_TICKET) {
                impl.fail(result, "TLS read failed");
            }
        }
        if (result == MBEDTLS_ERR_SSL_WANT_READ) {
            impl.waitForData(50ms);
        }
    }
}

void ATlsSession::write(std::span<const std::byte> source) {
    auto& impl = *mImpl;
    std::lock_guard lock(impl.mutex);
    while (!source.empty()) {
        const int result =
            mbedtls_ssl_write(&impl.ssl, reinterpret_cast<const unsigned char*>(source.data()), source.size());
        if (result > 0) {
            source = source.subspan(static_cast<std::size_t>(result));
        } else if (result != MBEDTLS_ERR_SSL_WANT_WRITE && result != MBEDTLS_ERR_SSL_WANT_READ) {
            impl.fail(result, "TLS write failed");
        }
    }
}

void ATlsSession::close() noexcept {
    std::lock_guard lock(mImpl->mutex);
    if (mImpl->closed) {
        return;
    }
    mImpl->closed = true;
    try {
        mbedtls_ssl_close_notify(&mImpl->ssl);
    } catch (...) {
    }
    mImpl->transportError = nullptr;
}

AOptional<AByteBuffer> ATlsSession::peerFingerprintSha256() const { return mImpl->peerFingerprint; }
