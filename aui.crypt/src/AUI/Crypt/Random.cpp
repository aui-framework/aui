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
// Created by alex2 on 25.09.2020.
//

#include "Random.h"

#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>
#include <AUI/Common/AException.h>

_<AByteBuffer> ACrypto::safeRandom() {
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context drbg;
    mbedtls_entropy_init(&entropy);
    mbedtls_ctr_drbg_init(&drbg);
    struct Cleanup {
        mbedtls_entropy_context& entropy;
        mbedtls_ctr_drbg_context& drbg;
        ~Cleanup() {
            mbedtls_ctr_drbg_free(&drbg);
            mbedtls_entropy_free(&entropy);
        }
    } cleanup { entropy, drbg };

    const char* personalization = "aui.crypt.safeRandom";
    if (mbedtls_ctr_drbg_seed(&drbg, mbedtls_entropy_func, &entropy,
                              reinterpret_cast<const unsigned char*>(personalization), strlen(personalization)) != 0) {
        throw AException("Failed to seed RNG");
    }

    constexpr size_t TOTAL_SIZE = 0x1000;
    auto bb = _new<AByteBuffer>();
    bb->resize(TOTAL_SIZE);
    auto* data = reinterpret_cast<unsigned char*>(bb->data());
    // a single call of mbedtls_ctr_drbg_random is limited to MBEDTLS_CTR_DRBG_MAX_REQUEST bytes
    for (size_t offset = 0; offset < TOTAL_SIZE; offset += MBEDTLS_CTR_DRBG_MAX_REQUEST) {
        const size_t chunk = std::min<size_t>(MBEDTLS_CTR_DRBG_MAX_REQUEST, TOTAL_SIZE - offset);
        if (mbedtls_ctr_drbg_random(&drbg, data + offset, chunk) != 0) {
            throw AException("Failed to gather random bytes");
        }
    }
    return bb;
}
