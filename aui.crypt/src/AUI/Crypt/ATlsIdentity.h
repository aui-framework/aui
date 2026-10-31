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
#include <memory>

#include <AUI/Common/AByteBuffer.h>
#include <AUI/Common/AString.h>
#include <AUI/Common/SharedPtrTypes.h>
#include <AUI/api.h>

/**
 * @brief Certificate with its private key, used by the server side of [ATlsSession].
 * @ingroup crypt
 * @details
 * Created with [ATlsIdentity::generateSelfSigned]. A self-signed certificate is not trusted by anyone, so a client
 * identifies the server by the certificate's fingerprint (`fingerprintSha256()`) obtained from a trusted
 * channel instead.
 *
 * ```cpp
 * auto identity = ATlsIdentity::generateSelfSigned("my-service");
 * ALogger::info("tls") << "fingerprint: " << identity->fingerprintHex();
 * ```
 *
 * Instances are immutable and may be shared between sessions and threads.
 */
class API_AUI_CRYPT ATlsIdentity {
public:
    ~ATlsIdentity();
    ATlsIdentity(const ATlsIdentity&) = delete;

    /**
     * @brief Generates an ECDSA (P-256) key and a self-signed certificate for it.
     * @param commonName subject of the certificate
     * @param validity how long the certificate is valid, starting from a day ago (tolerates unsynchronized clocks)
     */
    [[nodiscard]] static _<ATlsIdentity>
    generateSelfSigned(const AString& commonName, std::chrono::hours validity = std::chrono::hours(24 * 365));

    /**
     * @brief SHA-256 of the DER encoded certificate.
     */
    [[nodiscard]] const AByteBuffer& fingerprintSha256() const noexcept;

    /**
     * @brief fingerprintSha256 as lowercase hex without separators.
     */
    [[nodiscard]] AString fingerprintHex() const;

    /**
     * @brief Formats a fingerprint the same way as fingerprintHex.
     */
    [[nodiscard]] static AString toHex(AByteBufferView fingerprint);

private:
    friend class ATlsSession;
    struct Impl;
    explicit ATlsIdentity(std::unique_ptr<Impl> impl);
    std::unique_ptr<Impl> mImpl;
};
