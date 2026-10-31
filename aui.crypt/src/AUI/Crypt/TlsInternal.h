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

#include <cstddef>

#include <mbedtls/pk.h>
#include <mbedtls/x509_crt.h>

#include "ATlsIdentity.h"

/// Shared by the TLS sources; not part of the API.
namespace aui::tls {

/// Initializes the crypto subsystem of mbedTLS (once).
void init();

/// Random generator for mbedTLS (mbedtls_f_rng_t); thread-safe, the context argument is ignored.
int random(void*, unsigned char* output, std::size_t size);

/// mbedtls_strerror
AString describe(int code);

}   // namespace aui::tls

struct ATlsIdentity::Impl {
    mbedtls_pk_context key;
    mbedtls_x509_crt certificate;
    AByteBuffer fingerprint;

    Impl() {
        mbedtls_pk_init(&key);
        mbedtls_x509_crt_init(&certificate);
    }
    ~Impl() {
        mbedtls_x509_crt_free(&certificate);
        mbedtls_pk_free(&key);
    }
};
