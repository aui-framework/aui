/*
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

#pragma once


#include <AUI/Common/AByteBuffer.h>

/**
 * @brief Various hash functions
 * @ingroup crypt
 */
namespace AHash {
    [[nodiscard]] API_AUI_CRYPT AByteBuffer sha512(const AByteBuffer& in);
    [[nodiscard]] API_AUI_CRYPT AByteBuffer sha512(IInputStream& in);
    [[nodiscard]] inline AByteBuffer sha512(const _<IInputStream>& in) {
        return sha512(*in);
    }
    [[nodiscard]] inline AByteBuffer sha512(IInputStream&& in) {
        return sha512(in);
    }

    [[nodiscard]] API_AUI_CRYPT AByteBuffer sha256(const AByteBuffer& in);
    [[nodiscard]] API_AUI_CRYPT AByteBuffer sha256(IInputStream& in);
    [[nodiscard]] inline AByteBuffer sha256(const _<IInputStream>& in) {
        return sha256(*in);
    }
    [[nodiscard]] inline AByteBuffer sha256(IInputStream&& in) {
        return sha256(in);
    }

    [[nodiscard]] API_AUI_CRYPT AByteBuffer sha1(const AByteBuffer& in);
    [[nodiscard]] API_AUI_CRYPT AByteBuffer sha1(IInputStream& in);
    [[nodiscard]] inline AByteBuffer sha1(const _<IInputStream>& in) {
        return sha1(*in);
    }
    [[nodiscard]] inline AByteBuffer sha1(IInputStream&& in) {
        return sha1(in);
    }

    [[nodiscard]] API_AUI_CRYPT AByteBuffer md5(const AByteBuffer& in);
    [[nodiscard]] API_AUI_CRYPT AByteBuffer md5(IInputStream& in);
    [[nodiscard]] inline AByteBuffer md5(const _<IInputStream>& in) {
        return md5(*in);
    }
    [[nodiscard]] inline AByteBuffer md5(IInputStream&& in) {
        return md5(in);
    }
}


