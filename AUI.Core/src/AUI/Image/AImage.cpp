/**
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
 * Original code located at https://github.com/Alex2772/aui
 * =====================================================================================================================
 */

//
// Created by alex2772 on 25.07.2018.
//

#include <utility>
#include <cstring>
#include "AImage.h"
#include "AImageLoaderRegistry.h"

AImage::AImage() {

}

AImage::AImage(AImage::Format f) :
        mFormat(f)
{

}

AImage::AImage(AVector<uint8_t> mData, uint32_t mWidth, uint32_t mHeight, int mFormat) : mData(std::move(
        mData)), mWidth(mWidth), mHeight(mHeight), mFormat(mFormat) {

}

AVector<uint8_t>& AImage::getData() {
    return mData;
}

AImage AImage::addAlpha(const AImage& src)
{
    AImage dst(AVector<uint8_t>(), src.getWidth(), src.getHeight(), RGBA | BYTE);
    dst.getData().resize(src.getWidth() * src.getHeight() * dst.getBytesPerPixel());
    memset(dst.getData().data(), 255, dst.getData().size());

    // https://stackoverflow.com/questions/9900854/opengl-creating-texture-atlas-at-run-time
    for (int sourceY = 0; sourceY < src.mHeight; ++sourceY) {
        for (int sourceX = 0; sourceX < src.mWidth; ++sourceX) {
            int from = (sourceY * src.mWidth * src.getBytesPerPixel()) + (sourceX * src.getBytesPerPixel()); // 4 bytes per pixel (assuming RGBA)
            int to = ((sourceY)* dst.mWidth * dst.getBytesPerPixel()) + ((sourceX)* dst.getBytesPerPixel()); // same format as source

            for (int channel = 0; channel < src.getBytesPerPixel(); ++channel) {
                dst.mData[to + channel] = src.mData[from + channel];
            }
        }
    }
    return dst;
}

AImage AImage::resize(const AImage& src, uint32_t width, uint32_t height) {
    AImage n(AVector<uint8_t>(), width, height, src.getFormat());
    n.getData().resize(width * height * n.getBytesPerPixel());
    copy(src, n, 0, 0);
    return n;
}

glm::ivec4 AImage::getPixelAt(uint32_t x, uint32_t y) const
{
    const uint8_t* dataPtr = &mData[(mWidth * glm::clamp(y, uint32_t(0), mHeight) + glm::clamp(x, uint32_t(0), mWidth)) * getBytesPerPixel()];
    switch (getBytesPerPixel())
    {
        case 1:
            return { dataPtr[0], 0, 0, 0 };
        case 2:
            return { dataPtr[0], dataPtr[1], 0, 0 };
        case 3:
            return { dataPtr[0], dataPtr[1], dataPtr[2], 0 };
        case 4:
            return { dataPtr[0], dataPtr[1], dataPtr[2], dataPtr[3] };
    }
    return {};
}
void AImage::setPixelAt(uint32_t x, uint32_t y, const glm::ivec4& val)
{
    uint8_t* dataPtr = &mData[(mWidth * glm::clamp(y, uint32_t(0), mHeight) + glm::clamp(x, uint32_t(0), mWidth)) * getBytesPerPixel()];
    switch (getBytesPerPixel())
    {
        case 4:
            dataPtr[3] = val.w;
        case 3:
            dataPtr[2] = val.z;
        case 2:
            dataPtr[1] = val.y;
        case 1:
            dataPtr[0] = val.x;
    }
}

AImage AImage::resizeLinearDownscale(const AImage& src, uint32_t width, uint32_t height)
{
    AImage n(AVector<uint8_t>(), width, height, src.getFormat());
    n.getData().resize(width * height * n.getBytesPerPixel());

    uint32_t deltaX = src.getWidth() / width;
    uint32_t deltaY = src.getHeight() / height;

    for (uint32_t y = 0; y < height; ++y)
    {
        for (uint32_t x = 0; x < width; ++x)
        {
            glm::ivec4 block(0);
            for (uint32_t dy = 0; dy < deltaY; ++dy)
            {
                for (uint32_t dx = 0; dx < deltaX; ++dx)
                {
                    block += src.getPixelAt(x * deltaX + dx, y * deltaY + dy);
                }
            }
            block /= deltaY * deltaX;
            n.setPixelAt(x, y, block);
        }
    }

    return n;
}

void AImage::copy(const AImage& src, AImage& dst, uint32_t x, uint32_t y) {
    assert(src.getFormat() == dst.getFormat());

    // https://stackoverflow.com/questions/9900854/opengl-creating-texture-atlas-at-run-time
    for (uint32_t sourceY = 0; sourceY < glm::min(src.mHeight, dst.mHeight); ++sourceY) {
        for (uint32_t sourceX = 0; sourceX < glm::min(src.mWidth, dst.mWidth); ++sourceX) {
            uint32_t from = (sourceY * uint32_t(src.mWidth) * uint32_t(src.getBytesPerPixel())) + (sourceX * uint32_t(src.getBytesPerPixel())); // 4 bytes per pixel (assuming RGBA)
            uint32_t to = ((y + sourceY) *uint32_t(dst.mWidth) * uint32_t(dst.getBytesPerPixel())) + ((x + sourceX) * uint32_t(dst.getBytesPerPixel())); // same format as source

            for (uint32_t channel = 0; channel < dst.getBytesPerPixel(); ++channel) {
                dst.mData[to + channel] = src.mData[from + channel];
            }
        }
    }
}

_<AImage> AImage::fromUrl(const AUrl& url) {
    return AImageLoaderRegistry::inst().loadImage(url);
}

