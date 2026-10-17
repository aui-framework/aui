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

#include <AUI/Common/SharedPtrTypes.h>
#include <AUI/Common/AByteBuffer.h>
#include <AUI/Util/APimpl.h>
#include <AUI/api.h>

/**
 * @brief Stores public and/or private RSA key and encrypts/decrypts data with it.
 * @ingroup crypt
 * @details
 * Instances are created with one of the static factories: [ARsa::generate] creates a new key pair,
 * `fromPrivateKeyPEM` and `fromPublicKeyPEM` load an existing key. ARsa is not copyable; factories return `_<ARsa>`.
 *
 * Encryption uses PKCS#1 v1.5 padding. Input of any length is accepted: it is split to blocks of
 * `getKeyLength() - 11` bytes, each encrypted separately, and output blocks are `getKeyLength()` bytes long. Operations
 * failing inside mbedTLS throw `AException`.
 *
 * An instance loaded with `fromPublicKeyPEM` holds no private part, so it is meant for encryption only.
 *
 * See [crypt-rsa] for examples.
 *
 * !!! warning
 *
 *     Encryption does not authenticate data. There is no signing API in this class.
 */
class API_AUI_CRYPT ARsa : public aui::noncopyable {
private:
    class ARsaPrivate;
    // Sized for mbedtls_pk_context + mbedtls_entropy_context + mbedtls_ctr_drbg_context.
    // With MBEDTLS_THREADING_C the latter two each carry a mutex, so this needs headroom
    // over the raw context sizes.
    aui::fast_pimpl<ARsaPrivate, 1536> mPrivate;

    ARsa();

public:
    ~ARsa();

    /**
     * @brief Encrypts data with the public part of the key.
     * @param in data of arbitrary length.
     * @return concatenated encrypted blocks.
     */
    AByteBuffer encrypt(AByteBufferView in);

    /**
     * @brief Decrypts data produced by encrypt(). Requires the private part of the key.
     * @param in concatenated encrypted blocks.
     * @return original data.
     */
    AByteBuffer decrypt(AByteBufferView in);

    /**
     * @brief Returns the key length in bytes (modulus size).
     */
    size_t getKeyLength() const;

    /**
     * @brief Exports the private key in PEM format. Requires the private part of the key.
     */
    AByteBuffer getPrivateKeyPEM() const;

    /**
     * @brief Exports the public key in PEM format.
     */
    AByteBuffer getPublicKeyPEM() const;

    /**
     * @brief Generates a new key pair.
     * @param bits key size in bits; 2048 by default.
     */
    static _<ARsa> generate(int bits = 0x800);

    /**
     * @brief Loads a private key (and thus the public key) from PEM.
     * Throws `AException` if the data cannot be parsed.
     */
    static _<ARsa> fromPrivateKeyPEM(AByteBufferView buffer);

    /**
     * @brief Loads a public key from PEM.
     * Throws `AException` if the data cannot be parsed.
     */
    static _<ARsa> fromPublicKeyPEM(AByteBufferView buffer);
};
