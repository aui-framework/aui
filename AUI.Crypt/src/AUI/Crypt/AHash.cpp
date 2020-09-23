//
// Created by alex2 on 23.09.2020.
//

#include <cassert>
#include "AHash.h"
#include <openssl/sha.h>


template<typename Functor>
inline AByteBuffer sha_impl(Functor f, size_t s, const AByteBuffer& in) {
    AByteBuffer out;
    out.reserve(s);
    out.setSize(s);
    f((const unsigned char*)in.getCurrentPosAddress(), in.getAvailable(), (unsigned char*)out.data());
    return out;
}



AByteBuffer AHash::sha512(const AByteBuffer& in) {
    return sha_impl(SHA512, 64, in);
}
