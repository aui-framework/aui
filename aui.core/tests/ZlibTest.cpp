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
#include <range/v3/all.hpp>
#include "AUI/Common/AByteBuffer.h"
#include "AUI/IO/AFileInputStream.h"
#include "AUI/IO/AFileOutputStream.h"
#include "AUI/Util/LZ.h"
#include "AUI/Util/Archive.h"

#include <AUI/IO/AByteBufferInputStream.h>

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


TEST(Zlib, DecompressZip) {
    static constexpr uint8_t blob[] = {
        0x50, 0x4b, 0x03, 0x04, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0xa4, 0xb4, 0x39, 0x5a, 0x21, 0x17,
        0x93, 0x7d, 0x05, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x08, 0x00, 0x1c, 0x00, 0x74, 0x65,
        0x73, 0x74, 0x2e, 0x74, 0x78, 0x74, 0x55, 0x54, 0x09, 0x00, 0x03, 0x64, 0x3d, 0x95, 0x67, 0xbe,
        0x3d, 0x95, 0x67, 0x75, 0x78, 0x0b, 0x00, 0x01, 0x04, 0xf5, 0x01, 0x00, 0x00, 0x04, 0x00, 0x00,
        0x00, 0x00, 0x31, 0x32, 0x33, 0x34, 0x0a, 0x50, 0x4b, 0x01, 0x02, 0x1e, 0x03, 0x0a, 0x00, 0x00,
        0x00, 0x00, 0x00, 0xa4, 0xb4, 0x39, 0x5a, 0x21, 0x17, 0x93, 0x7d, 0x05, 0x00, 0x00, 0x00, 0x05,
        0x00, 0x00, 0x00, 0x08, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0xa4,
        0x81, 0x00, 0x00, 0x00, 0x00, 0x74, 0x65, 0x73, 0x74, 0x2e, 0x74, 0x78, 0x74, 0x55, 0x54, 0x05,
        0x00, 0x03, 0x64, 0x3d, 0x95, 0x67, 0x75, 0x78, 0x0b, 0x00, 0x01, 0x04, 0xf5, 0x01, 0x00, 0x00,
        0x04, 0x00, 0x00, 0x00, 0x00, 0x50, 0x4b, 0x05, 0x06, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01,
        0x00, 0x4e, 0x00, 0x00, 0x00, 0x47, 0x00, 0x00, 0x00, 0x00, 0x00 };
    // test.txt: 1234

    bool called = false;
    aui::archive::zip::read(AByteBufferInputStream(AByteBufferView::fromRaw(blob)), [&](const aui::archive::FileEntry& e) {
        EXPECT_FALSE(called);
        called = true;
        EXPECT_EQ(e.name, "test.txt");
        EXPECT_EQ(AString(AByteBuffer::fromStream(e.open()), AStringEncoding::LATIN1), "1234\n");
    });
    EXPECT_TRUE(called);
}

TEST(Zlib, CompressZip) {
    {
        aui::archive::zip::Writer writer(std::make_unique<AFileOutputStream>("test.zip"));
        writer.openFileInZip(APath("test") / "test.txt", [](IOutputStream& os) {
            os << std::string_view("1234\n");
        });
    }

    bool called = false;
    aui::archive::zip::read(AFileInputStream("test.zip"), [&](const aui::archive::FileEntry& e) {
        EXPECT_FALSE(called);
        called = true;
        EXPECT_EQ(e.name, "test/test.txt");
        EXPECT_EQ(AString(AByteBuffer::fromStream(e.open()), AStringEncoding::LATIN1), "1234\n");
    });
    EXPECT_TRUE(called);
}


