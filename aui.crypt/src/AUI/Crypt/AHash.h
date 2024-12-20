/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
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

#pragma once


#include <AUI/Common/AByteBuffer.h>

/**
 * @brief Various hash functions
 * @ingroup crypt
 */
namespace AHash {
    [[nodiscard]] API_AUI_CRYPT AByteBuffer sha512(AByteBufferView in);
    [[nodiscard]] API_AUI_CRYPT AByteBuffer sha512(aui::no_escape<IInputStream> in);

    [[nodiscard]] API_AUI_CRYPT AByteBuffer sha256(AByteBufferView in);
    [[nodiscard]] API_AUI_CRYPT AByteBuffer sha256(aui::no_escape<IInputStream> in);

    [[nodiscard]] API_AUI_CRYPT AByteBuffer sha1(AByteBufferView in);
    [[nodiscard]] API_AUI_CRYPT AByteBuffer sha1(aui::no_escape<IInputStream> in);

    [[nodiscard]] API_AUI_CRYPT AByteBuffer md5(AByteBufferView in);
    [[nodiscard]] API_AUI_CRYPT AByteBuffer md5(aui::no_escape<IInputStream> in);

    [[nodiscard]] API_AUI_CRYPT AByteBuffer sha256hmac(AByteBufferView in, AByteBufferView key);
}


