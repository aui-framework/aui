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
// Created by Alex2772 on 9/23/2022.
//

#include "Win32Util.h"
#include "AUI/Util/ARaiiHelper.h"

AImage aui::win32::iconToImage(HICON hIcon) {
    assert(hIcon != nullptr);
    ICONINFO iconInfo;
    GetIconInfo(hIcon, &iconInfo);
    HBITMAP hBitmap = iconInfo.hbmColor;

    ARaiiHelper r = [&] {
        DeleteObject(iconInfo.hbmColor);
        DeleteObject(iconInfo.hbmMask);
    };

    return bitmapToImage(hBitmap);
}

AImage aui::win32::bitmapToImage(HBITMAP hbitmap) {
    BITMAP bmp = {0};
    BITMAPINFO info = {0};
    AByteBuffer pixels;

    HDC compatDC = CreateCompatibleDC(nullptr);
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
    DeleteDC(compatDC);
    AImage image(std::move(pixels), width, height, AImage::RGBA | AImage::BYTE);
    return {image.imageDataOfFormat(AImage::BGRA | AImage::BYTE | AImage::FLIP_Y),
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height),
            AImage::RGBA | AImage::BYTE };
}
