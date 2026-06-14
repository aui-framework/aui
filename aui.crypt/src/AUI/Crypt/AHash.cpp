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

#include "AHash.h"

#include <cassert>
#include <mbedtls/sha512.h>
#include <mbedtls/sha256.h>
#include <mbedtls/sha1.h>
#include <mbedtls/md5.h>
#include <mbedtls/md.h>
#include <AUI/IO/IInputStream.h>

template<typename CTX, typename FInit, typename FStarts, typename FUpdate, typename FFinish, typename FFree>
static AByteBuffer hash_impl(FInit init, FStarts starts, FUpdate update, FFinish finish, FFree free, size_t s, AByteBufferView in) {
    AByteBuffer out;
    out.reserve(s);
    out.setSize(s);

    CTX ctx;
    init(&ctx);
    starts(&ctx);
    update(&ctx, reinterpret_cast<const unsigned char*>(in.data()), in.size());
    finish(&ctx, reinterpret_cast<unsigned char*>(out.data()));
    free(&ctx);

    return out;
}


template<typename CTX, typename FInit, typename FStarts, typename FUpdate, typename FFinish, typename FFree>
static AByteBuffer hash_impl(FInit init, FStarts starts, FUpdate update, FFinish finish, FFree free, size_t s, aui::no_escape<IInputStream> in) {
    AByteBuffer result;
    result.reserve(s);
    result.setSize(s);

    char tmp[0x10000];

    CTX ctx;
    init(&ctx);
    starts(&ctx);
    for (size_t r; (r = in->read(tmp, sizeof(tmp))) > 0;) {
        update(&ctx, reinterpret_cast<const unsigned char*>(tmp), r);
    }
    finish(&ctx, reinterpret_cast<unsigned char*>(result.data()));
    free(&ctx);
    return result;
}

AByteBuffer AHash::sha512(AByteBufferView in) {
    return hash_impl<mbedtls_sha512_context>(
        mbedtls_sha512_init,
        [](mbedtls_sha512_context* ctx) { mbedtls_sha512_starts(ctx, 0); },
        mbedtls_sha512_update,
        mbedtls_sha512_finish,
        mbedtls_sha512_free,
        64,
        in
    );
}

AByteBuffer AHash::sha512(aui::no_escape<IInputStream> in) {
    return hash_impl<mbedtls_sha512_context>(
        mbedtls_sha512_init,
        [](mbedtls_sha512_context* ctx) { mbedtls_sha512_starts(ctx, 0); },
        mbedtls_sha512_update,
        mbedtls_sha512_finish,
        mbedtls_sha512_free,
        64,
        in
    );
}

AByteBuffer AHash::sha256(AByteBufferView in) {
    return hash_impl<mbedtls_sha256_context>(
        mbedtls_sha256_init,
        [](mbedtls_sha256_context* ctx) { mbedtls_sha256_starts(ctx, 0); },
        mbedtls_sha256_update,
        mbedtls_sha256_finish,
        mbedtls_sha256_free,
        32,
        in
    );
}

AByteBuffer AHash::sha256(aui::no_escape<IInputStream> in) {
    return hash_impl<mbedtls_sha256_context>(
        mbedtls_sha256_init,
        [](mbedtls_sha256_context* ctx) { mbedtls_sha256_starts(ctx, 0); },
        mbedtls_sha256_update,
        mbedtls_sha256_finish,
        mbedtls_sha256_free,
        32,
        in
    );
}

AByteBuffer AHash::sha1(AByteBufferView in) {
    return hash_impl<mbedtls_sha1_context>(
        mbedtls_sha1_init,
        mbedtls_sha1_starts,
        mbedtls_sha1_update,
        mbedtls_sha1_finish,
        mbedtls_sha1_free,
        20,
        in
    );
}

AByteBuffer AHash::sha1(aui::no_escape<IInputStream> in) {
    return hash_impl<mbedtls_sha1_context>(
        mbedtls_sha1_init,
        mbedtls_sha1_starts,
        mbedtls_sha1_update,
        mbedtls_sha1_finish,
        mbedtls_sha1_free,
        20,
        in
    );
}

AByteBuffer AHash::md5(AByteBufferView in) {
    return hash_impl<mbedtls_md5_context>(
        mbedtls_md5_init,
        mbedtls_md5_starts,
        mbedtls_md5_update,
        mbedtls_md5_finish,
        mbedtls_md5_free,
        16,
        in
    );
}

AByteBuffer AHash::md5(aui::no_escape<IInputStream> in) {
    return hash_impl<mbedtls_md5_context>(
        mbedtls_md5_init,
        mbedtls_md5_starts,
        mbedtls_md5_update,
        mbedtls_md5_finish,
        mbedtls_md5_free,
        16,
        in
    );
}

AByteBuffer AHash::sha256hmac(AByteBufferView in, AByteBufferView key) {
    AByteBuffer result;
    result.resize(32);

    const mbedtls_md_info_t* md_info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);

    mbedtls_md_hmac(
        md_info,
        reinterpret_cast<const unsigned char*>(key.data()),
        key.size(),
        reinterpret_cast<const unsigned char*>(in.data()),
        in.size(),
        reinterpret_cast<unsigned char*>(result.data())
    );

    return result;
}
