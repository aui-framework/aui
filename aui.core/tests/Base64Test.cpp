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

#include <gtest/gtest.h>
#include <AUI/Common/AByteBuffer.h>

TEST(Base64, EncodeDecode) {
    std::string_view text = "Building beautiful programs in pure C++ without chromium embedded framework";
    AByteBuffer buffer(text.data(), text.size());
    auto base64 = buffer.toBase64String();
    EXPECT_EQ(base64, "QnVpbGRpbmcgYmVhdXRpZnVsIHByb2dyYW1zIGluIHB1cmUgQysrIHdpdGhvdXQgY2hyb21pdW0gZW1iZWRkZWQgZnJhbWV3b3Jr");
    auto decoded = AString::fromUtf8(AByteBuffer::fromBase64String(base64));
    EXPECT_EQ(decoded, text);
}
