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
// Created by alex2 on 23.09.2020.
//

#pragma once


#include <AUI/Common/AByteBuffer.h>

/**
 * @defgroup crypt aui::crypt
 * @brief Cryptography: hashing, HMAC and RSA.
 * @details
 * aui::crypt is a small module built on top of mbedTLS. It provides:
 *
 * - message digests (MD5, SHA-1, SHA-256, SHA-512) and HMAC-SHA256, see [crypt-hashing];
 * - RSA key generation, PEM import/export and encryption, see [ARsa] and [crypt-rsa];
 * - a weak helper for gathering random bytes, see [crypt-random].
 *
 * Results are returned as [AByteBuffer], which can be converted to hex or base64 text with `toHexString()` and
 * `toBase64String()`.
 *
 * To use the module, link it to your target:
 *
 * ```cpp
 * auib_import(aui https://github.com/aui-framework/aui COMPONENTS core crypt VERSION ${AUI_VERSION})
 * aui_link(${PROJECT_NAME} PRIVATE aui::crypt)
 * ```
 *
 * !!! note
 *
 *     The module does not cover symmetric ciphers (AES etc...), signatures or TLS. HTTPS requests are handled by
 *     [curl].
 */

/**
 * @brief Message digests (MD5, SHA-1, SHA-256, SHA-512) and HMAC-SHA256.
 * @ingroup crypt
 * @details
 * Each digest has two overloads: one takes an in-memory buffer, the other consumes an input stream until its end, so
 * large files can be hashed without loading them into memory. The result is the raw digest (16, 20, 32 and 64 bytes
 * for md5, sha1, sha256 and sha512 respectively). See [crypt-hashing] for examples.
 *
 * !!! warning
 *
 *     MD5 and SHA-1 are provided for compatibility with existing formats and protocols. Do not rely on them for
 *     security purposes. Plain digests are also not suitable for storing passwords.
 */
namespace AHash {
    [[nodiscard]] API_AUI_CRYPT AByteBuffer sha512(AByteBufferView in);
    [[nodiscard]] API_AUI_CRYPT AByteBuffer sha512(aui::no_escape<IInputStream> in);

    [[nodiscard]] API_AUI_CRYPT AByteBuffer sha256(AByteBufferView in);
    [[nodiscard]] API_AUI_CRYPT AByteBuffer sha256(aui::no_escape<IInputStream> in);

    [[nodiscard]] API_AUI_CRYPT AByteBuffer sha1(AByteBufferView in);
    [[nodiscard]] API_AUI_CRYPT AByteBuffer sha1(aui::no_escape<IInputStream> in);

    [[nodiscard]] API_AUI_CRYPT AByteBuffer md5(AByteBufferView in);
    [[nodiscard]] API_AUI_CRYPT AByteBuffer md5(aui::no_escape<IInputStream> in);

    [[nodiscard]] API_AUI_CRYPT AByteBuffer sha256hmac(AByteBufferView in, AByteBufferView key);
}


