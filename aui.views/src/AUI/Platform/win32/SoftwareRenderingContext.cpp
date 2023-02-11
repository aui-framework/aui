// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2023 Alex2772
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

#include <AUI/Platform/SoftwareRenderingContext.h>
#include <AUI/Software/SoftwareRenderer.h>

SoftwareRenderingContext::SoftwareRenderingContext()
{
}

SoftwareRenderingContext::~SoftwareRenderingContext() {

}

void SoftwareRenderingContext::init(const IRenderingContext::Init& init) {
    CommonRenderingContext::init(init);
    if (Render::getRenderer() == nullptr) {
        Render::setRenderer(std::make_unique<SoftwareRenderer>());
    }
}

void SoftwareRenderingContext::destroyNativeWindow(ABaseWindow& window) {
    CommonRenderingContext::destroyNativeWindow(window);
}

void SoftwareRenderingContext::beginPaint(ABaseWindow& window) {
    CommonRenderingContext::beginPaint(window);
    std::memset(mStencilBlob.data(), 0, mStencilBlob.getSize());
    for (size_t i = 0; i < mBitmapSize.x * mBitmapSize.y; ++i) {
        auto dataPtr = reinterpret_cast<uint32_t*>(mBitmapBlob.data() + sizeof(BITMAPINFO) + i * 4);
        *dataPtr = 0;
    }
}

void SoftwareRenderingContext::endPaint(ABaseWindow& window) {
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

void SoftwareRenderingContext::beginResize(ABaseWindow& window) {
}

void SoftwareRenderingContext::endResize(ABaseWindow& window) {
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
    return {std::move(data), mBitmapSize.x, mBitmapSize.y, AImageFormat::RGBA | AImageFormat::BYTE};
}
