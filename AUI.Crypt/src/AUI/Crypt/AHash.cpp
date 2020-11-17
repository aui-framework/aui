//
// Created by alex2 on 23.09.2020.
//

#include <cassert>
#include "AHash.h"
#include <openssl/sha.h>
#include <AUI/IO/IInputStream.h>


template<typename Functor>
inline AByteBuffer sha_impl(Functor f, size_t s, const AByteBuffer& in) {
    AByteBuffer out;
    out.reserve(s);
    out.setSize(s);
    f((const unsigned char*)in.getCurrentPosAddress(), in.getAvailable(), (unsigned char*)out.data());
    return out;
}


template<typename CTX, typename FInit, typename FUpdate, typename FFinal>
inline AByteBuffer sha_impl(FInit init, FUpdate update, FFinal final, size_t s, const _<IInputStream>& in) {
    AByteBuffer result;
    result.reserve(s);
    result.setSize(s);

    char tmp[0x10000];

    CTX ctx;
    init(&ctx);
    for (int r; (r = in->read(tmp, sizeof(tmp))) > 0;) {
        update(&ctx, tmp, r);
    }
    final((unsigned char*) result.data(), &ctx);
    return result;
}



AByteBuffer AHash::sha512(const AByteBuffer& in) {
    return sha_impl(SHA512, 64, in);
}

AByteBuffer AHash::sha512(const _<IInputStream>& in) {
    return sha_impl<SHA512_CTX>(SHA512_Init, SHA512_Update, SHA512_Final, 64, in);
}

AByteBuffer AHash::sha256(const AByteBuffer& in) {
    return sha_impl(SHA256, 32, in);
}

AByteBuffer AHash::sha256(const _<IInputStream>& in) {
    return sha_impl<SHA256_CTX>(SHA256_Init, SHA256_Update, SHA256_Final, 32, in);
}


AByteBuffer AHash::sha1(const AByteBuffer& in) {
    return sha_impl(SHA1, 20, in);
}

AByteBuffer AHash::sha1(const _<IInputStream>& in) {
    return sha_impl<SHA_CTX>(SHA1_Init, SHA1_Update, SHA1_Final, 20, in);
}
