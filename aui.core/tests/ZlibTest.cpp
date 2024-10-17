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

#include <gtest/gtest.h>
#include <range/v3/all.hpp>
#include "AUI/Common/AByteBuffer.h"
#include "AUI/Util/LZ.h"

namespace {
AByteBuffer makeSource() {
    AByteBuffer source;
    source.resize(128);
    ranges::generate(source, [i = 0]() mutable {
        return i++ / 10;
    });
    return source;
}
}

TEST(Zlib, ActuallyCompresses) {
    auto source = makeSource();

    AByteBuffer compressed;
    aui::zlib::compress(source, compressed);

    EXPECT_LE(compressed.size(), source.size());

    AByteBuffer decompressed;
    aui::zlib::decompress(compressed, decompressed);

    EXPECT_EQ(source, decompressed);
}

TEST(Zlib, DecompressStream) {
    auto source = makeSource();

    AByteBuffer compressed;
    aui::zlib::compress(source, compressed);

    auto decompressed = AByteBuffer::fromStream(aui::zlib::decompressToStream(compressed));

    EXPECT_EQ(source, decompressed);
}
