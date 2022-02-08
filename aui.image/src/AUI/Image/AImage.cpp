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

 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
 */

//
// Created by Алексей on 25.07.2018.
//

#include <utility>
#include <cstring>
#include "AImage.h"
#include "AImageLoaderRegistry.h"
#include <stdexcept>

AImage::AImage()
{
}

AImage::AImage(AVector<std::uint8_t> mData, uint32_t mWidth, uint32_t mHeight, int mFormat) : mData(std::move(
        mData)), mWidth(mWidth), mHeight(mHeight), mFormat(mFormat)
{
}

AImage::AImage(uint32_t width, uint32_t height, int format):
        mWidth(width),
        mHeight(height),
        mFormat(format)
{
    mData.resize(mWidth * mHeight * getBytesPerPixel());
}


AImage AImage::addAlpha(const AImage& src)
{
    AImage dst(AVector<std::uint8_t>(), src.getWidth(), src.getHeight(), RGBA | BYTE);
    dst.getData().resize(src.getWidth() * src.getHeight() * dst.getBytesPerPixel());
    memset(dst.getData().data(), 255, dst.getData().size());

    // https://stackoverflow.com/questions/9900854/opengl-creating-texture-atlas-at-run-time
    for (int sourceY = 0; sourceY < src.mHeight; ++sourceY)
    {
        for (int sourceX = 0; sourceX < src.mWidth; ++sourceX)
        {
            int from = (sourceY * src.mWidth * src.getBytesPerPixel()) + (sourceX * src.getBytesPerPixel());
            // 4 bytes per pixel (assuming RGBA)
            int to = ((sourceY) * dst.mWidth * dst.getBytesPerPixel()) + ((sourceX) * dst.getBytesPerPixel()); // same format as source

            for (int channel = 0; channel < src.getBytesPerPixel(); ++channel)
            {
                dst.mData[to + channel] = src.mData[from + channel];
            }
        }
    }
    return dst;
}

AImage AImage::resize(const AImage& src, uint32_t width, uint32_t height)
{
    AImage n(width, height, src.getFormat());
    n.getData().resize(width * height * n.getBytesPerPixel());
    copy(src, n, 0, 0);
    return n;
}

glm::ivec4 AImage::getPixelAt(uint32_t x, uint32_t y) const
{
    const std::uint8_t* dataPtr = &mData[(mWidth * glm::clamp(y, static_cast<uint32_t>(0), mHeight - 1) + glm::clamp(
            x, static_cast<uint32_t>(0), mWidth - 1)) * getBytesPerPixel()];
    switch (getBytesPerPixel())
    {
        case 1:
            return {dataPtr[0], 0, 0, 0};
        case 2:
            return {dataPtr[0], dataPtr[1], 0, 0};
        case 3:
            return {dataPtr[0], dataPtr[1], dataPtr[2], 0};
        case 4:
            return {dataPtr[0], dataPtr[1], dataPtr[2], dataPtr[3]};
    }
    return {};
}

void AImage::setPixelAt(uint32_t x, uint32_t y, const glm::ivec4& val)
{
    std::uint8_t* dataPtr = &mData[(mWidth * glm::clamp(y, static_cast<uint32_t>(0), mHeight - 1) + glm::clamp(
            x, static_cast<uint32_t>(0), mWidth - 1)) * getBytesPerPixel()];
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
    AImage n(AVector<std::uint8_t>(), width, height, src.getFormat());
    n.getData().resize(width * height * n.getBytesPerPixel());

    uint32_t deltaX = src.getWidth() / width;
    uint32_t deltaY = src.getHeight() / height;

    for (uint32_t y = 0; y < height; ++y)
    {
        for (uint32_t x = 0; x < width; ++x)
        {
            glm::ivec4 block(0.f);
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

void AImage::copy(const AImage& src, AImage& dst, uint32_t x, uint32_t y)
{
    assert(src.getFormat() == dst.getFormat());

    // https://stackoverflow.com/questions/9900854/opengl-creating-texture-atlas-at-run-time
    for (uint32_t sourceY = 0; sourceY < (glm::min)(src.mHeight, dst.mHeight); ++sourceY)
    {
        for (uint32_t sourceX = 0; sourceX < (glm::min)(src.mWidth, dst.mWidth); ++sourceX)
        {
            uint32_t from = (sourceY * src.mWidth * src.getBytesPerPixel()) + (sourceX * src.getBytesPerPixel());
            // 4 bytes per pixel (assuming RGBA)
            uint32_t to = ((y + sourceY) * dst.mWidth * dst.getBytesPerPixel()) + ((x + sourceX) * dst.getBytesPerPixel());
            // same format as source

            for (uint32_t channel = 0; channel < dst.getBytesPerPixel(); ++channel)
            {
                dst.mData[to + channel] = src.mData[from + channel];
            }
        }
    }
}

AImage AImage::sub(uint32_t x, uint32_t y, uint32_t width, uint32_t height) const
{
    AImage image(width, height, getFormat());


    for (uint32_t sourceY = 0; sourceY < height; ++sourceY)
    {
        for (uint32_t sourceX = 0; sourceX < width; ++sourceX)
        {
            uint32_t from = ((y + sourceY) * mWidth + (x + sourceX)) * getBytesPerPixel();
            uint32_t to = (sourceY * width + sourceX) * getBytesPerPixel();

            for (uint32_t channel = 0; channel < getBytesPerPixel(); ++channel)
            {
                image.mData[to + channel] = mData[from + channel];
            }
        }
    }

    return image;
}

/*!
 * @return Количество байт на пиксель.
 */
std::uint8_t AImage::getBytesPerPixel() const
{
    std::uint8_t b = static_cast<std::uint8_t>(mFormat & 15);
    if (mFormat & FLOAT)
    {
        b *= 4;
    }
    return b;
}

_<AImage> AImage::fromUrl(const AUrl& url) {
    return Cache::get(url);
}

_<AImage> AImage::fromFile(const APath& path) {
    return fromUrl(AUrl("file://" + path));
}

_<AImage> AImage::Cache::load(const AUrl& key)
{
    try {
        auto buffer = AByteBuffer::fromStream(AUrl(key).open(), 0x100000);

        if (auto raster = AImageLoaderRegistry::inst().loadRaster(buffer))
            return raster;
    } catch (const AException& e) {
        ALogger::err("Could not load image: " + key.getFull() + ": " + e.getMessage());
    }
    return nullptr;
}

AImage::Cache& AImage::Cache::inst() {
    static AImage::Cache s;
    return s;
}


