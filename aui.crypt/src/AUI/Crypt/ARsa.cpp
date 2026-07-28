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

#include "ARsa.h"

#include <mbedtls/rsa.h>
#include <mbedtls/pk.h>
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/pem.h>
#include <mbedtls/des.h>
#include <mbedtls/error.h>
#include <AUI/Common/AException.h>
#include <glm/glm.hpp>

constexpr uint8_t gKey[] = { 52, 12, 96, 112, 84, 4, 95, 125, 213, 234, 87, 195, 175, 13, 42, 0 };

class ARsa::ARsaPrivate {
public:
    mbedtls_pk_context pk = {};
    mbedtls_entropy_context entropy = {};
    mbedtls_ctr_drbg_context ctr_drbg = {};

    ARsaPrivate() {
        mbedtls_pk_init(&pk);
        mbedtls_entropy_init(&entropy);
        mbedtls_ctr_drbg_init(&ctr_drbg);

        const char* pers = "aui.crypt";
        if (mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,
                                   reinterpret_cast<const uint8_t*>(pers), strlen(pers)) != 0) {
            throw AException("Failed to seed RNG");
        }
    }

    ~ARsaPrivate() {
        mbedtls_pk_free(&pk);
        mbedtls_ctr_drbg_free(&ctr_drbg);
        mbedtls_entropy_free(&entropy);
    }

    mbedtls_rsa_context* getRsaContext() const {
        return mbedtls_pk_rsa(pk);
    }
};

ARsa::ARsa() {}

ARsa::~ARsa() {}

AByteBuffer ARsa::encrypt(AByteBufferView in) {
    AByteBuffer buf;
    buf.reserve(in.size() + 0x1000);

    auto rsa = mPrivate->getRsaContext();
    size_t keyLen = mbedtls_rsa_get_len(rsa);
    size_t paddingSize = 11; // PKCS#1 v1.5 padding size

    for (auto it = in.begin(); it != in.end();) {
        auto toRead = glm::min(static_cast<size_t>(in.end() - it), keyLen - paddingSize);

        size_t oldSize = buf.getSize();
        buf.setSize(oldSize + keyLen);

        int r = mbedtls_rsa_pkcs1_encrypt(
            rsa,
            mbedtls_ctr_drbg_random,
            &mPrivate->ctr_drbg,
            toRead,
            reinterpret_cast<const unsigned char*>(it),
            reinterpret_cast<unsigned char*>(buf.data() + oldSize)
        );

        if (r != 0) {
            char error_buf[100];
            mbedtls_strerror(r, error_buf, sizeof(error_buf));
            throw AException("Could not encrypt: {}"_format(error_buf));
        }

        it += toRead;
    }

    return buf;
}

AByteBuffer ARsa::decrypt(AByteBufferView in) {
    AByteBuffer buf;
    buf.reserve(in.size() + 0x1000);

    auto rsa = mPrivate->getRsaContext();
    size_t keyLen = mbedtls_rsa_get_len(rsa);

    for (auto it = in.begin(); it != in.end();) {
        auto toRead = glm::min(static_cast<size_t>(in.end() - it), keyLen);

        unsigned char temp[4096];
        size_t olen = 0;

        int r = mbedtls_rsa_pkcs1_decrypt(
            rsa,
            mbedtls_ctr_drbg_random,
            &mPrivate->ctr_drbg,
            &olen,
            reinterpret_cast<const unsigned char*>(it),
            temp,
            sizeof(temp)
        );

        if (r != 0) {
            char error_buf[64];
            mbedtls_strerror(r, error_buf, sizeof(error_buf));
            throw AException("Could not decrypt: {}"_format(error_buf));
        }

        size_t oldSize = buf.getSize();
        buf.setSize(oldSize + olen);
        memcpy(buf.data() + oldSize, temp, olen);

        it += toRead;
    }

    return buf;
}

size_t ARsa::getKeyLength() const {
    return mbedtls_rsa_get_len(mPrivate->getRsaContext());
}

AByteBuffer ARsa::getPrivateKeyPEM() const {
    std::vector<uint8_t> output(16000);
    int ret = mbedtls_pk_write_key_pem(&mPrivate->pk, output.data(), output.size());

    if (ret != 0) {
        char error_buf[64];
        mbedtls_strerror(ret, error_buf, sizeof(error_buf));
        throw AException("Could not write private key: {}"_format(error_buf));
    }

    AByteBuffer byteBuffer;
    size_t len = strlen(reinterpret_cast<const char*>(output.data()));
    byteBuffer.write(reinterpret_cast<const char*>(output.data()), len);

    return byteBuffer;
}

AByteBuffer ARsa::getPublicKeyPEM() const {
    std::vector<uint8_t> output(16000);
    int ret = mbedtls_pk_write_pubkey_pem(&mPrivate->pk, output.data(), output.size());

    if (ret != 0) {
        char error_buf[64];
        mbedtls_strerror(ret, error_buf, sizeof(error_buf));
        throw AException("Could not write public key: {}"_format(error_buf));
    }

    AByteBuffer byteBuffer;
    size_t len = strlen(reinterpret_cast<const char*>(output.data()));
    byteBuffer.write(reinterpret_cast<const char*>(output.data()), len);

    return byteBuffer;
}

_<ARsa> ARsa::generate(int bits) {
    auto instance = aui::ptr::manage_shared(new ARsa());

    int ret = mbedtls_pk_setup(&instance->mPrivate->pk,
                                mbedtls_pk_info_from_type(MBEDTLS_PK_RSA));
    if (ret != 0) {
        char error_buf[64];
        mbedtls_strerror(ret, error_buf, sizeof(error_buf));
        throw AException("mbedtls_pk_setup failed: {}"_format(error_buf));
    }

    ret = mbedtls_rsa_gen_key(
        instance->mPrivate->getRsaContext(),
        mbedtls_ctr_drbg_random,
        &instance->mPrivate->ctr_drbg,
        bits,
        65537 // RSA_F4
    );

    if (ret != 0) {
        char error_buf[64];
        mbedtls_strerror(ret, error_buf, sizeof(error_buf));
        throw AException("mbedtls_rsa_gen_key failed: {}"_format(error_buf));
    }

    return instance;
}

_<ARsa> ARsa::fromPrivateKeyPEM(AByteBufferView buffer) {
    auto instance = aui::ptr::manage_shared(new ARsa());

    AByteBuffer nullTerminated;
    nullTerminated.write(buffer.data(), buffer.size());
    nullTerminated.write("\0", 1);

    int ret = mbedtls_pk_parse_key(
        &instance->mPrivate->pk,
        reinterpret_cast<const unsigned char*>(nullTerminated.data()),
        nullTerminated.size(),
        gKey,
        sizeof(gKey) - 1,
        mbedtls_ctr_drbg_random,
        &instance->mPrivate->ctr_drbg
    );

    if (ret != 0) {
        char error_buf[64];
        mbedtls_strerror(ret, error_buf, sizeof(error_buf));
        throw AException("Could not parse RSA private key: {}"_format(error_buf));
    }

    return instance;
}

_<ARsa> ARsa::fromPublicKeyPEM(AByteBufferView buffer) {
    auto instance = aui::ptr::manage_shared(new ARsa());

    AByteBuffer nullTerminated;
    nullTerminated.write(buffer.data(), buffer.size());
    nullTerminated.write("\0", 1);

    int ret = mbedtls_pk_parse_public_key(
        &instance->mPrivate->pk,
        reinterpret_cast<const unsigned char*>(nullTerminated.data()),
        nullTerminated.size()
    );

    if (ret != 0) {
        char error_buf[100];
        mbedtls_strerror(ret, error_buf, sizeof(error_buf));
        throw AException("Could not parse RSA public key: {}"_format(error_buf));
    }

    return instance;
}
