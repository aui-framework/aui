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


#include <utility>
#include <cstring>
#include "AImage.h"
#include "AImageLoaderRegistry.h"
#include <stdexcept>
#include <AUI/Traits/memory.h>

AImage::AImage()
{
}

AImage::AImage(uint32_t width, uint32_t height, AImageFormat format):
        mWidth(width),
        mHeight(height),
        mFormat(format)
{
    mData.resize(mWidth * mHeight * getBytesPerPixel());
}


AImage AImage::resize(const AImage& src, uint32_t width, uint32_t height)
{
    AImage n(width, height, src.getFormat());
    n.getData().resize(width * height * n.getBytesPerPixel());
    copy(src, n, 0, 0);
    return n;
}


void AImage::setPixelAt(std::uint32_t index, glm::ivec4 color) {
    std::uint8_t* dataPtr = &mData.at<std::uint8_t>(index * getBytesPerPixel());
    switch (getBytesPerPixel())
    {
        case 4:
            dataPtr[3] = color.w;
        case 3:
            dataPtr[2] = color.z;
        case 2:
            dataPtr[1] = color.y;
        case 1:
            dataPtr[0] = color.x;
    }
}

void AImage::setPixelAt(uint32_t x, uint32_t y, const glm::ivec4& val)
{
    setPixelAt(mWidth * glm::clamp(y, static_cast<uint32_t>(0), mHeight - 1) + glm::clamp(
            x, static_cast<uint32_t>(0), mWidth - 1), val);
}

AImage AImage::resizeLinearDownscale(const AImage& src, uint32_t width, uint32_t height)
{
    AImage n(width, height, src.getFormat());

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
                    block += src.getPixelAt(glm::uvec2{x * deltaX + dx, y * deltaY + dy});
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
                dst.mData.at<std::uint8_t>(to + channel) = src.mData.at<std::uint8_t>(from + channel);
            }
        }
    }
}

AImage AImage::sub(uint32_t x, uint32_t y, uint32_t width, uint32_t height) const
{
    assert(x + width <= mWidth);
    assert(y + height <= mHeight);
    AImage image(width, height, getFormat());


    for (uint32_t sourceY = 0; sourceY < height; ++sourceY)
    {
        for (uint32_t sourceX = 0; sourceX < width; ++sourceX)
        {
            uint32_t from = ((y + sourceY) * mWidth + (x + sourceX)) * getBytesPerPixel();
            uint32_t to = (sourceY * width + sourceX) * getBytesPerPixel();

            for (uint32_t channel = 0; channel < getBytesPerPixel(); ++channel)
            {
                image.mData.at<std::uint8_t>(to + channel) = mData.at<std::uint8_t>(from + channel);
            }
        }
    }

    return image;
}


_<AImage> AImage::fromUrl(const AUrl& url) {
    return Cache::get(url);
}

_<AImage> AImage::fromFile(const APath& path) {
    return fromUrl(AUrl("file://" + path));
}

_<AImage> AImage::fromBuffer(AByteBufferView buffer) {
    try {
        if (auto raster = AImageLoaderRegistry::inst().loadRaster(buffer))
            return raster;
    } catch (const AException& e) {
        ALogger::err("Could not load image: " + e.getMessage());
    }
    return nullptr;
}

_<AImage> AImage::Cache::load(const AUrl& key)
{
    try {
        auto buffer = AByteBuffer::fromStream(AUrl(key).open(), 0x100000);

        if (auto raster = AImageLoaderRegistry::inst().loadRaster(buffer))
            return raster;
    } catch (const AException& e) {
        ALogger::err("Could not load image: " + key.full() + ": " + e.getMessage());
    }
    return nullptr;
}

AImage::Cache& AImage::Cache::inst() {
    static AImage::Cache s;
    return s;
}


AColor AImage::averageColor() const noexcept {
    if (mWidth == 0 || mHeight == 0) {
        return AColor::BLACK;
    }

    glm::ivec4 accumulator;
    aui::zero(accumulator);

    for (uint32_t y = 0; y < mHeight; ++y) {
        for (uint32_t x = 0; x < mWidth; ++x) {
            accumulator += getPixelAt({x, y});
        }
    }

    accumulator /= mWidth * mHeight;

    return glm::vec4(accumulator) / 255.f;
}

AByteBuffer AImage::imageDataOfFormat(unsigned format) const {
    assert(("unsupported", (format & AImageFormat::BYTE)));

    bool flipY = format & AImageFormat::FLIP_Y;
    format &= ~AImageFormat::FLIP_Y;

    AByteBuffer output;
    for (std::uint32_t iY = 0; iY < getHeight(); ++iY) {
        int y;
        if (flipY) {
            y = getHeight() - iY - 1;
        } else {
            y = iY;
        }

        for (std::uint32_t x = 0; x < getWidth(); ++x) {
            auto color = getPixelAt({x, y});
            switch (format & ~AImageFormat::BYTE) {
                case AImageFormat::R:
                    output << std::uint8_t(color.r);
                    break;

                case AImageFormat::RG:
                    output << std::uint8_t(color.r);
                    output << std::uint8_t(color.g);
                    break;

                case AImageFormat::RGB:
                    output << std::uint8_t(color.r);
                    output << std::uint8_t(color.g);
                    output << std::uint8_t(color.b);
                    break;

                case AImageFormat::RGBA:
                    output << std::uint8_t(color.r);
                    output << std::uint8_t(color.g);
                    output << std::uint8_t(color.b);
                    output << std::uint8_t(color.a);
                    break;

                case AImageFormat::BGRA:
                    output << std::uint8_t(color.b);
                    output << std::uint8_t(color.g);
                    output << std::uint8_t(color.r);
                    output << std::uint8_t(color.a);
                    break;

                case AImageFormat::ARGB:
                    output << std::uint8_t(color.a);
                    output << std::uint8_t(color.r);
                    output << std::uint8_t(color.g);
                    output << std::uint8_t(color.b);
                    break;

                default:
                    assert(0);
            }
        }
    }
    return output;
}

void AImage::mirrorVertically() {
    for (std::uint32_t y = 0; y < getHeight() / 2; ++y) {
        auto mirroredY = getHeight() - y - 1;
        auto l1 = getData().begin() + getBytesPerPixel() * getWidth() * y;
        auto l2 = getData().begin() + getBytesPerPixel() * getHeight() * mirroredY;
        for (std::uint32_t x = 0; x < getWidth() * getBytesPerPixel(); ++x, ++l1, ++l2) {
            std::swap(*l1, *l2);
        }
    }
}

void AImage::fillColor(glm::ivec4 c) {
    const auto s = mData.size() / getBytesPerPixel();
    for (std::size_t i = 0; i < s; ++i) {
        setPixelAt(i, c);
    }
}
