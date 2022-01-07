/**
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
 */

//
// Created by alex2 on 23.09.2020.
//

#include <cassert>
#include "AHash.h"
#include <openssl/sha.h>
#include <openssl/md5.h>
#include <AUI/IO/IInputStream.h>


template<typename Functor>
inline AByteBuffer sha_impl(Functor f, size_t s, const AByteBuffer& in) {
    AByteBuffer out;
    out.reserve(s);
    out.setSize(s);
    f((const unsigned char*)in.data(), in.getSize(), (unsigned char*)out.data());
    return out;
}


template<typename CTX, typename FInit, typename FUpdate, typename FFinal>
inline AByteBuffer sha_impl(FInit init, FUpdate update, FFinal final, size_t s, IInputStream& in) {
    AByteBuffer result;
    result.reserve(s);
    result.setSize(s);

    char tmp[0x10000];

    CTX ctx;
    init(&ctx);
    for (int r; (r = in.read(tmp, sizeof(tmp))) > 0;) {
        update(&ctx, tmp, r);
    }
    final((unsigned char*) result.data(), &ctx);
    return result;
}



AByteBuffer AHash::sha512(const AByteBuffer& in) {
    return sha_impl(SHA512, 64, in);
}

AByteBuffer AHash::sha512(IInputStream& in) {
    return sha_impl<SHA512_CTX>(SHA512_Init, SHA512_Update, SHA512_Final, 64, in);
}

AByteBuffer AHash::sha256(const AByteBuffer& in) {
    return sha_impl(SHA256, 32, in);
}

AByteBuffer AHash::sha256(IInputStream& in) {
    return sha_impl<SHA256_CTX>(SHA256_Init, SHA256_Update, SHA256_Final, 32, in);
}


AByteBuffer AHash::sha1(const AByteBuffer& in) {
    return sha_impl(SHA1, 20, in);
}

AByteBuffer AHash::sha1(IInputStream& in) {
    return sha_impl<SHA_CTX>(SHA1_Init, SHA1_Update, SHA1_Final, 20, in);
}

AByteBuffer AHash::md5(const AByteBuffer& in) {
    return sha_impl(MD5, 16, in);
}

AByteBuffer AHash::md5(IInputStream& in) {
    return sha_impl<MD5_CTX>(MD5_Init, MD5_Update, MD5_Final, 16, in);
}
