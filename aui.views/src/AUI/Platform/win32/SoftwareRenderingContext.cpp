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

#include <AUI/Platform/SoftwareRenderingContext.h>
#include <AUI/Software/SoftwareRenderer.h>

SoftwareRenderingContext::SoftwareRenderingContext()
{
}

SoftwareRenderingContext::~SoftwareRenderingContext() {
}

void SoftwareRenderingContext::init(const IRenderingContext::Init& init) {
    CommonRenderingContext::init(init);
}

void SoftwareRenderingContext::destroyNativeWindow(ASurface& window) {
    CommonRenderingContext::destroyNativeWindow(window);
}

void SoftwareRenderingContext::beginPaint(ASurface& window) {
    CommonRenderingContext::beginPaint(window);
    std::memset(mStencilBlob.data(), 0, mStencilBlob.getSize());
    for (size_t i = 0; i < mBitmapSize.x * mBitmapSize.y; ++i) {
        auto dataPtr = reinterpret_cast<uint32_t*>(mBitmapBlob.data() + sizeof(BITMAPINFO) + i * 4);
        *dataPtr = 0;
    }
}

void SoftwareRenderingContext::endPaint(ASurface& window) {
    if (mPainterDC != 0) {
        StretchDIBits(mPainterDC,
                      0, 0,
                      mBitmapSize.x, mBitmapSize.y,
                      0, 0,
                      mBitmapSize.x, mBitmapSize.y,
                      mBitmapInfo->bmiColors,
                      mBitmapInfo,
                      DIB_RGB_COLORS,
                      SRCCOPY);
    }
    CommonRenderingContext::endPaint(window);
}

void SoftwareRenderingContext::beginResize(ASurface& window) {
}

void SoftwareRenderingContext::endResize(ASurface& window) {
    reallocate(window);
}

void SoftwareRenderingContext::reallocate(const ASurface &window) {
    mBitmapSize = window.getSize();
    mBitmapBlob.reallocate(mBitmapSize.x * mBitmapSize.y * 4 + sizeof(*mBitmapInfo));
    mStencilBlob.reallocate(mBitmapSize.x * mBitmapSize.y);
    mBitmapInfo = reinterpret_cast<BITMAPINFO*>(mBitmapBlob.data());

    mBitmapInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    mBitmapInfo->bmiHeader.biPlanes = 1;
    mBitmapInfo->bmiHeader.biBitCount = 32;
    mBitmapInfo->bmiHeader.biCompression = BI_RGB;

    mBitmapInfo->bmiHeader.biWidth = mBitmapSize.x;
    mBitmapInfo->bmiHeader.biHeight = -int(mBitmapSize.y); // negative means top-down bitmap
}

void SoftwareRenderingContext::reallocate() {

}

AImage SoftwareRenderingContext::makeScreenshot() {
    AByteBuffer data;
    size_t s = mBitmapSize.x * mBitmapSize.y * 4;
    data.resize(s);
    for (size_t i = 0; i < s; i += 4) {
        uint8_t* ptr = reinterpret_cast<uint8_t*>(mBitmapBlob.data() + sizeof(BITMAPINFOHEADER) + i);
        data.at<std::uint8_t>(i    ) = ptr[2];
        data.at<std::uint8_t>(i + 1) = ptr[1];
        data.at<std::uint8_t>(i + 2) = ptr[0];
        data.at<std::uint8_t>(i + 3) = ptr[3];
    }
    return {std::move(data), mBitmapSize, APixelFormat::RGBA | APixelFormat::BYTE};
}

IRenderer& SoftwareRenderingContext::renderer() {
    static SoftwareRenderer r;
    return r;
}
