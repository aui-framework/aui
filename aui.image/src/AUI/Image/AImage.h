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

public:
    AImage(uint32_t width, uint32_t height, int format);

    enum Format : std::uint32_t {
        UNKNOWN = 0,
        BYTE = 0b1,
        FLOAT = 0b10,
        R    = 0b00100,
        RG   = 0b01000,
        RGB  = 0b01100,
        RGBA = 0b10000,
        ARGB = 0b10100,
        BGRA = 0b11000,


        FLIP_Y = 0b10000000,


    };

    enum Meta : std::uint32_t {
        META = 0b11,
        STRUCTURE = ~META
    };
private:
    AByteBuffer mData;
    uint32_t mWidth;
    uint32_t mHeight;
    unsigned mFormat = UNKNOWN;

public:
    AImage();

    AImage(unsigned int format) : mFormat(format) {}

    AImage(AByteBuffer data,
           uint32_t width,
           uint32_t height,
           unsigned int format):
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
    uint8_t getBytesPerPixel() const;


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

    [[nodiscard]]
    glm::ivec4 getPixelAt(uint32_t x, uint32_t y) const;
    void setPixelAt(uint32_t x, uint32_t y, const glm::ivec4& val);

    void mirrorVertically();

    [[nodiscard]]
    static AImage addAlpha(const AImage& AImage);

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

