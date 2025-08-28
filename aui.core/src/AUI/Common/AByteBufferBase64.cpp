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
// Created by Alex2772 on 7/13/2022.
//

#include <AUI/Common/AByteBuffer.h>
#include <AUI/Common/AByteBufferView.h>
#include <simdutf.h>

AString AByteBufferView::toBase64String() const {
    size_t result_length = simdutf::base64_length_from_binary(size());
    AString result(result_length, '\0');
    simdutf::binary_to_base64(data(), size(), reinterpret_cast<char*>(result.bytes().data()));
    return result;
}

AByteBuffer AByteBuffer::fromBase64String(const AStringView& base64) {
    AByteBuffer buffer(simdutf::maximal_binary_length_from_base64(base64.data(), base64.size()));
    simdutf::result r = simdutf::base64_to_binary(base64.data(), base64.size(), buffer.data());
    if (r.error) {
        return {};
    }
    buffer.resize(r.count);
    return buffer;
}
