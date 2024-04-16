// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2024 Alex2772 and Contributors
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

aui::win32::Bitmap aui::win32::imageRgbToBitmap(const AImage& image, BitmapMode mode) {
    aui::win32::DeviceContext hdcScreen = GetDC(nullptr);

    aui::win32::DeviceContext hdcMemColor = CreateCompatibleDC(hdcScreen);


    BITMAPINFO bmi;
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = image.width();
    bmi.bmiHeader.biHeight = image.height();
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biSizeImage = bmi.bmiHeader.biWidth * bmi.bmiHeader.biHeight * 4;

    bmi.bmiHeader.biHeight = -bmi.bmiHeader.biHeight;

    VOID* data;
    auto hbmpColor = CreateDIBSection(hdcMemColor, &bmi, DIB_RGB_COLORS, &data, nullptr, 0x0);
    auto hbmpOldColor = (HBITMAP)SelectObject(hdcMemColor, hbmpColor);

    if (image.format() == (APixelFormat::BGRA | APixelFormat::BYTE)) {
        std::memcpy(data, image.buffer().data(), bmi.bmiHeader.biSizeImage);
    } else {
        for (int y = 0; y < image.height(); ++y) {
            for (int x = 0; x < image.width(); ++x) {
                reinterpret_cast<AFormattedColor<APixelFormat::BGRA | APixelFormat::BYTE>*>(data)[y * image.width() + x] = AFormattedColorConverter(image.get({x, y}));
            }
        }
    }

    SelectObject(hdcMemColor, hbmpOldColor);


    return hbmpColor;
}

AImage aui::win32::bitmapToImage(HBITMAP hbitmap) {
    BITMAP bmp = {0};
    BITMAPINFO info = {0};
    AByteBuffer pixels;

    aui::win32::DeviceContext compatDC = CreateCompatibleDC(nullptr);
    std::memset(&info, 0, sizeof(BITMAPINFO));
    auto oldBitmap = (HBITMAP)SelectObject(compatDC, hbitmap);
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
    GetDIBits(compatDC, hbitmap, 0, height, pixels.data(), &info, DIB_RGB_COLORS);
    SelectObject(compatDC, oldBitmap);

    height = std::abs(height);
    AImage image(std::move(pixels), {width, height}, APixelFormat::BGRA | APixelFormat::BYTE);
    image.mirrorVertically();
    return image;
}
