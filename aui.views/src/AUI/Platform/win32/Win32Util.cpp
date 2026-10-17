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
// Created by Alex2772 on 9/23/2022.
//

#include "Win32Util.h"
#include "AUI/Util/ARaiiHelper.h"

AImage aui::win32::iconToImage(HICON hIcon) {
    AUI_ASSERT(hIcon != nullptr);
    ICONINFO iconInfo;
    GetIconInfo(hIcon, &iconInfo);
    HBITMAP hBitmap = iconInfo.hbmColor;

    ARaiiHelper r = [&] {
        DeleteObject(iconInfo.hbmColor);
        DeleteObject(iconInfo.hbmMask);
    };

    return bitmapToImage(hBitmap);
}

aui::win32::Bitmap aui::win32::imageToBitmap(AImageView image, BitmapAlpha alpha) {
    BITMAPINFO bmi {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = image.width();
    bmi.bmiHeader.biHeight = -image.height(); // negative = top-down rows
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biSizeImage = image.width() * image.height() * 4;

    void* data = nullptr;
    Bitmap bitmap(CreateDIBSection(nullptr, &bmi, DIB_RGB_COLORS, &data, nullptr, 0));
    if (!bitmap || !data) {
        return nullptr;
    }

    using Bgra = AFormattedColor<APixelFormat::BGRA | APixelFormat::BYTE>;
    auto* dst = static_cast<Bgra*>(data);
    if (image.format() == (APixelFormat::BGRA | APixelFormat::BYTE)) {
        std::memcpy(data, image.buffer().data(), bmi.bmiHeader.biSizeImage);
    } else {
        for (int y = 0; y < image.height(); ++y) {
            for (int x = 0; x < image.width(); ++x) {
                dst[y * image.width() + x] = AFormattedColorConverter(image.get({ x, y }));
            }
        }
    }

    if (alpha == BitmapAlpha::PREMULTIPLIED) {
        auto* bytes = static_cast<std::uint8_t*>(data);
        for (std::size_t i = 0; i < std::size_t(image.width()) * image.height() * 4; i += 4) {
            const int a = bytes[i + 3];
            bytes[i + 0] = std::uint8_t((bytes[i + 0] * a + 127) / 255);
            bytes[i + 1] = std::uint8_t((bytes[i + 1] * a + 127) / 255);
            bytes[i + 2] = std::uint8_t((bytes[i + 2] * a + 127) / 255);
        }
    }
    return bitmap;
}

namespace {
aui::win32::Icon createIcon(AImageView image, bool isIcon, glm::ivec2 hotspot) {
    const auto color = aui::win32::imageToBitmap(image);

    // the mask is ignored by the system when the color bitmap carries alpha; it still has to be a valid bitmap.
    const std::vector<std::uint8_t> maskBits(((image.width() + 15) / 16) * 2 * image.height(), 0);
    const aui::win32::Bitmap mask(CreateBitmap(image.width(), image.height(), 1, 1, maskBits.data()));

    ICONINFO info {};
    info.fIcon = isIcon;
    info.xHotspot = hotspot.x;
    info.yHotspot = hotspot.y;
    info.hbmMask = mask.get();
    info.hbmColor = color.get();
    return aui::win32::Icon(CreateIconIndirect(&info));
}
}   // namespace

aui::win32::Icon aui::win32::imageToIcon(AImageView image) {
    return createIcon(image, true, {});
}

aui::win32::Icon aui::win32::imageToCursor(AImageView image, glm::ivec2 hotspot) {
    return createIcon(image, false, hotspot);
}

AImage aui::win32::bitmapToImage(HBITMAP hbitmap) {
    BITMAP bmp = {0};
    BITMAPINFO info = {0};
    AByteBuffer pixels;

    const aui::win32::MemoryDc compatDC(CreateCompatibleDC(nullptr));
    std::memset(&info, 0, sizeof(BITMAPINFO));
    auto oldBitmap = (HBITMAP)SelectObject(compatDC.get(), hbitmap);
    GetObject(hbitmap, sizeof(bmp), &bmp);

    info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    int width, height;
    info.bmiHeader.biWidth = width = bmp.bmWidth;
    info.bmiHeader.biHeight = height = bmp.bmHeight;
    info.bmiHeader.biPlanes = 1;
    info.bmiHeader.biBitCount = 32;
    info.bmiHeader.biCompression = BI_RGB;
    info.bmiHeader.biSizeImage = ((width * 32 + 31) / 32) * 4 * height;

    pixels.resize(info.bmiHeader.biSizeImage);
    GetDIBits(compatDC.get(), hbitmap, 0, height, pixels.data(), &info, DIB_RGB_COLORS);
    SelectObject(compatDC.get(), oldBitmap);

    height = std::abs(height);
    AImage image(std::move(pixels), {width, height}, APixelFormat::BGRA | APixelFormat::BYTE);
    image.mirrorVertically();
    return image;
}
