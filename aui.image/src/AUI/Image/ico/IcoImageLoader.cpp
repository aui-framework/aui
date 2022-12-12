// AUI Framework - Declarative UI toolkit for modern C++17
// Copyright (C) 2020-2022 Alex2772
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

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
    assert(("sizes could not be empty", !images.empty()));

    Header header;
    header.count = images.size();
    *os << header;

    std::size_t offset = sizeof(header) + sizeof(IconEntry) * images.size();

    auto bitmaps = images.map([](const AImage& image) {
        AByteBuffer buffer(image.getData().size() * 3);
        BitmapHeader bitmapHeader;
        bitmapHeader.sizeOfStructure = sizeof(bitmapHeader);
        bitmapHeader.width = image.getWidth();
        bitmapHeader.height = image.getHeight() * 2;
        bitmapHeader.bitsPerPixel = image.getBytesPerPixel() * 8;
        bitmapHeader.imageSize = image.getWidth() * image.getHeight() * 2;

        buffer << bitmapHeader;

        auto data = image.imageDataOfFormat(AImageFormat::FLIP_Y | AImageFormat::BYTE | (image.getBytesPerPixel() == 4 ? AImageFormat::BGRA : AImageFormat::RGB));

        buffer << AByteBufferView { (const char*)data.data(), data.size() };

        AVector<std::uint8_t> mask(image.getWidth() * image.getHeight() / 8, 0xff);
        buffer << AByteBufferView { (const char*)mask.data(), mask.size() };
        return buffer;
    });

    for (const auto&[image, buffer] : aui::zip(images, bitmaps)) {
        assert(("ico does not support images bigger than 256 px side", glm::all(glm::lessThanEqual(image.getSize(), glm::ivec2(256)))));
        IconEntry iconEntry {
                static_cast<uint8_t>(image.getWidth() == 256 ? 0 : image.getWidth()),
                static_cast<uint8_t>(image.getHeight() == 256 ? 0 : image.getHeight()),
                0,
                0,
                1,
                static_cast<uint16_t>(image.getBytesPerPixel() * 8),
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
