// AUI Framework - Declarative UI toolkit for modern C++17
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

//
// Created by alex2772 on 25.07.2018.
//

#pragma once

#include <cstdint>
#include <glm/glm.hpp>
#include <AUI/Common/AVector.h>
#include <AUI/Common/AByteBuffer.h>
#include <AUI/Util/Cache.h>
#include <AUI/Url/AUrl.h>
#include <AUI/IO/APath.h>
#include "AUI/Common/AColor.h"
#include "AImageFormat.h"

/**
 * @brief Image representation.
 * @ingroup image
 * @details
 * TODO refactor!
 */
class API_AUI_IMAGE AImage {
private:
    class Cache;
    friend class ::Cache<AImage, Cache, AUrl>;
    friend class AImageLoaderRegistry;
    class Cache: public ::Cache<AImage, Cache, AUrl> {
    public:
        static Cache& inst();
    protected:
        _<AImage> load(const AUrl& key) override;
    };

    void setPixelAt(std::uint32_t index, glm::ivec4 color);

private:
    AByteBuffer mData;
    uint32_t mWidth;
    uint32_t mHeight;
    AImageFormat mFormat = AImageFormat::UNKNOWN;


    std::size_t coordToIndex(glm::uvec2 coords) const noexcept {
        return mWidth * glm::clamp(coords.y, static_cast<uint32_t>(0), mHeight - 1) + glm::clamp(
                coords.x, static_cast<uint32_t>(0), mWidth - 1);
    }

public:

    AImage();

    AImage(AImageFormat format) : mFormat(format) {}


    AImage(uint32_t width, uint32_t height, AImageFormat format);

    AImage(AByteBuffer data,
           uint32_t width,
           uint32_t height,
           AImageFormat format):
            mData(std::move(data)),
            mWidth(width),
            mHeight(height),
            mFormat(format) {}

    void allocate() {
        mData.resize(mWidth * mHeight * getBytesPerPixel());
    }
    void setSize(const glm::uvec2& size) {
        mWidth = size.x;
        mHeight = size.y;
        allocate();
    }

    void fillColor(glm::ivec4 c);

    [[nodiscard]]
    AByteBuffer imageDataOfFormat(unsigned format) const;

    [[nodiscard]]
    AByteBuffer& getData() {
        return mData;
    }

    [[nodiscard]]
    const AByteBuffer& getData() const {
        return mData;
    }

    [[nodiscard]]
    inline uint32_t getWidth() const {
        return mWidth;
    }
    [[nodiscard]]
    inline uint32_t getHeight() const {
        return mHeight;
    }

    [[nodiscard]]
    inline unsigned getFormat() const {
        return mFormat;
    }

    /**
     * @return bytes per pixel.
     */
    [[nodiscard]]
    uint8_t getBytesPerPixel() const {
        return mFormat.getBytesPerPixel();
    }


    [[nodiscard]]
    inline glm::ivec2 getSize() const {
        return {getWidth(), getHeight()};
    }

    [[nodiscard]]
    AImage sub(uint32_t x, uint32_t y, uint32_t width, uint32_t height) const;

    [[nodiscard]]
    AImage sub(glm::ivec2 position, glm::ivec2 size) const {
        return sub(position.x, position.y, size.x, size.y);
    }

    void setPixelAt(uint32_t x, uint32_t y, const glm::ivec4& val);

    template<auto imageFormat = AImageFormat::RGBA | AImageFormat::BYTE>
    typename aui::image_format::traits<imageFormat>::pixel_t getPixelAt(glm::uvec2 coords) const noexcept {
        return aui::image_format::convert<imageFormat>(mFormat, (std::uint8_t*)mData.data() + getBytesPerPixel() * coordToIndex(coords));
    }

    void mirrorVertically();


    [[nodiscard]]
    static AImage resize(const AImage& src, uint32_t width, uint32_t height);

    [[nodiscard]]
    static AImage resizeLinearDownscale(const AImage& src, uint32_t width, uint32_t height);
    static void copy(const AImage& src, AImage& dst, uint32_t x, uint32_t y);

    [[nodiscard]]
    uint8_t& at(uint32_t x, uint32_t y) {
        return mData.at<std::uint8_t>((y * getWidth() + x) * getBytesPerPixel());
    }

    [[nodiscard]]
    const uint8_t& at(uint32_t x, uint32_t y) const {
        return mData.at<std::uint8_t>((y * getWidth() + x) * getBytesPerPixel());
    }

    [[nodiscard]]
    AColor averageColor() const noexcept;

    [[nodiscard]]
    static _<AImage> fromUrl(const AUrl& url);

    [[nodiscard]]
    static _<AImage> fromFile(const APath& path);

    [[nodiscard]]
    static _<AImage> fromBuffer(AByteBufferView buffer);
};

