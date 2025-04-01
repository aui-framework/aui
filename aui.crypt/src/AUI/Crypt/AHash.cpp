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

#include <cassert>
#include "AHash.h"
#include <openssl/sha.h>
#include <openssl/md5.h>
#include <openssl/hmac.h>
#include <AUI/IO/IInputStream.h>


template<typename Functor>
inline AByteBuffer sha_impl(Functor f, size_t s, AByteBufferView in) {
    AByteBuffer out;
    out.reserve(s);
    out.setSize(s);
    f((const unsigned char*)in.data(), in.size(), (unsigned char*)out.data());
    return out;
}


template<typename CTX, typename FInit, typename FUpdate, typename FFinal>
inline AByteBuffer sha_impl(FInit init, FUpdate update, FFinal final, size_t s, aui::no_escape<IInputStream> in) {
    AByteBuffer result;
    result.reserve(s);
    result.setSize(s);

    char tmp[0x10000];

    CTX ctx;
    init(&ctx);
    for (size_t r; (r = in->read(tmp, sizeof(tmp))) > 0;) {
        update(&ctx, tmp, r);
    }
    final((unsigned char*) result.data(), &ctx);
    return result;
}



AByteBuffer AHash::sha512(AByteBufferView in) {
    return sha_impl(SHA512, 64, in);
}

AByteBuffer AHash::sha512(aui::no_escape<IInputStream> in) {
    return sha_impl<SHA512_CTX>(SHA512_Init, SHA512_Update, SHA512_Final, 64, in);
}

AByteBuffer AHash::sha256(AByteBufferView in) {
    return sha_impl(SHA256, 32, in);
}

AByteBuffer AHash::sha256(aui::no_escape<IInputStream> in) {
    return sha_impl<SHA256_CTX>(SHA256_Init, SHA256_Update, SHA256_Final, 32, in);
}


AByteBuffer AHash::sha1(AByteBufferView in) {
    return sha_impl(SHA1, 20, in);
}

AByteBuffer AHash::sha1(aui::no_escape<IInputStream> in) {
    return sha_impl<SHA_CTX>(SHA1_Init, SHA1_Update, SHA1_Final, 20, in);
}

AByteBuffer AHash::md5(AByteBufferView in) {
    return sha_impl(MD5, 16, in);
}

AByteBuffer AHash::md5(aui::no_escape<IInputStream> in) {
    return sha_impl<MD5_CTX>(MD5_Init, MD5_Update, MD5_Final, 16, in);
}



template<typename Functor>
inline AByteBuffer hmac_impl(Functor f, size_t s, AByteBufferView in) {
    AByteBuffer out;
    out.reserve(s);
    out.setSize(s);
    f((const unsigned char*)in.data(), in.size(), (unsigned char*)out.data());
    return out;
}


template<typename CTX, typename FInit, typename FUpdate, typename FFinal>
inline AByteBuffer hmac_impl(FInit init, FUpdate update, FFinal final, size_t s, aui::no_escape<IInputStream> in) {
    AByteBuffer result;
    result.reserve(s);
    result.setSize(s);

    char tmp[0x10000];

    CTX ctx;
    init(&ctx);
    for (size_t r; (r = in->read(tmp, sizeof(tmp))) > 0;) {
        update(&ctx, tmp, r);
    }
    final((unsigned char*) result.data(), &ctx);
    return result;
}

AByteBuffer AHash::sha256hmac(AByteBufferView in, AByteBufferView key) {
    AByteBuffer result;
    result.resize(EVP_MAX_MD_SIZE);

    unsigned int size;
    HMAC(EVP_sha256(),
         key.data(),
         key.size(),
         reinterpret_cast<const unsigned char*>(in.data()),
         in.size(),
         reinterpret_cast<unsigned char*>(result.data()),
         &size);
    result.setSize(size);
    return result;
}
