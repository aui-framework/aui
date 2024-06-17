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
// Created by Alex2772 on 7/15/2022.
//

#include "IcoImageLoader.h"
#include <AUI/Common/AByteBufferView.h>
#include <AUI/Common/AByteBuffer.h>
#include <AUI/Traits/iterators.h>

namespace {
    struct Header {
        std::uint16_t _reserved0 = 0;
        enum Type : std::uint16_t {
            ICO = 1,
            CUR = 2,
        } type = ICO;
        std::uint16_t count;
    };
    static_assert(sizeof(Header) == 6, "ico header size should be exact 6 bytes");


    struct IconEntry {
        std::uint8_t width;
        std::uint8_t height;
        std::uint8_t colorPaletteSize = 0;
        std::uint8_t _reserved0 = 0;
        std::uint16_t colorPlanes = 1;
        std::uint16_t bitsPerPixel = 32;
        std::uint32_t size;
        std::uint32_t offset;
    };
    static_assert(sizeof(IconEntry) == 16, "ico entry header size should be exact 16 bytes");


    struct BitmapHeader {
        std::uint32_t sizeOfStructure;
        std::uint32_t width;
        std::uint32_t height;
        std::uint16_t planes = 1;
        std::uint16_t bitsPerPixel;
        std::uint32_t compression = 0;
        std::uint32_t imageSize;
        std::uint32_t _unused0 = 0;
        std::uint32_t _unused1 = 0;
        std::uint32_t _unused2 = 0;
        std::uint32_t _unused3 = 0;
    };
    static_assert(sizeof(BitmapHeader) == 40, "bmp header invalid size");
}

template<>
struct ASerializable<Header>: aui::raw_serializable<Header> {};
template<>
struct ASerializable<BitmapHeader>: aui::raw_serializable<BitmapHeader> {};

template<>
struct ASerializable<IconEntry>: aui::raw_serializable<IconEntry> {};

void IcoImageLoader::save(aui::no_escape<IOutputStream> os, const AVector<AImage>& images) {
    AUI_ASSERTX(!images.empty(), "sizes could not be empty");

    Header header;
    header.count = images.size();
    *os << header;

    std::size_t offset = sizeof(header) + sizeof(IconEntry) * images.size();

    auto bitmaps = images.map([](const AImage& image) {
        AByteBuffer buffer(image.buffer().size() * 3);
        BitmapHeader bitmapHeader;
        bitmapHeader.sizeOfStructure = sizeof(bitmapHeader);
        bitmapHeader.width = image.width();
        bitmapHeader.height = image.height() * 2;
        bitmapHeader.bitsPerPixel = image.bytesPerPixel() * 8;
        bitmapHeader.imageSize = image.width() * image.height() * 2;

        buffer << bitmapHeader;

        auto converted = image.convert(APixelFormat::BYTE | (image.bytesPerPixel() == 4 ? APixelFormat::BGRA : APixelFormat::RGB));
        converted.mirrorVertically();
        buffer << converted.buffer();

        AVector<std::uint8_t> mask(image.width() * image.height() / 8, 0xff);
        buffer << AByteBufferView { (const char*)mask.data(), mask.size() };
        return buffer;
    });

    for (const auto&[image, buffer] : aui::zip(images, bitmaps)) {
        AUI_ASSERTX(glm::all(glm::lessThanEqual(image.size(), glm::uvec2(256))), "ico does not support images bigger than 256 px side");
        IconEntry iconEntry {
                static_cast<uint8_t>(image.width() == 256 ? 0 : image.width()),
                static_cast<uint8_t>(image.height() == 256 ? 0 : image.height()),
                0,
                0,
                1,
                static_cast<uint16_t>(image.bytesPerPixel() * 8),
                static_cast<uint32_t>(buffer.size()),
                static_cast<uint32_t>(offset)
        };
        *os << iconEntry;
        offset += buffer.size();
    }

    for (const auto& bitmap : bitmaps) {
        *os << bitmap;
    }
}
