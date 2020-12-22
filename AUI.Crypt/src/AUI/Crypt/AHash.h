//
// Created by alex2 on 23.09.2020.
//

#pragma once


#include <AUI/Common/AByteBuffer.h>

namespace AHash {
    API_AUI_CRYPT AByteBuffer sha512(const AByteBuffer& in);
    API_AUI_CRYPT AByteBuffer sha512(const _<IInputStream>& in);

    API_AUI_CRYPT AByteBuffer sha256(const AByteBuffer& in);
    API_AUI_CRYPT AByteBuffer sha256(const _<IInputStream>& in);

    API_AUI_CRYPT AByteBuffer sha1(const AByteBuffer& in);
    API_AUI_CRYPT AByteBuffer sha1(const _<IInputStream>& in);

    API_AUI_CRYPT AByteBuffer md5(const AByteBuffer& in);
    API_AUI_CRYPT AByteBuffer md5(const _<IInputStream>& in);
}


