/*
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
// Created by alex2772 on 25.07.2018.
//

#pragma once

#include <cstdint>
#include <glm/glm.hpp>
#include <AUI/Common/AVector.h>
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

    enum Format : unsigned {
        UNKNOWN = 0,
        R = 1,
        RG = 2,
        RGB = 3,
        RGBA = 4,
        FLOAT = 8,
        BYTE = 16
    };
private:
    AVector<uint8_t> mData;
    uint32_t mWidth;
    uint32_t mHeight;
    unsigned mFormat = UNKNOWN;

public:
    AImage();

    AImage(unsigned int format) : mFormat(format) {}

    AImage(AVector<uint8_t> mData, uint32_t mWidth, uint32_t mHeight, int mFormat);

    void allocate() {
        mData.resize(mWidth * mHeight * getBytesPerPixel());
    }
    void setSize(const glm::uvec2& size) {
        mWidth = size.x;
        mHeight = size.y;
        allocate();
    }

    [[nodiscard]]
    AVector<uint8_t>& getData() {
        return mData;
    }

    [[nodiscard]]
    const AVector<uint8_t>& getData() const {
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


    [[nodiscard]]
    static AImage addAlpha(const AImage& AImage);

    [[nodiscard]]
    static AImage resize(const AImage& src, uint32_t width, uint32_t height);

    [[nodiscard]]
    static AImage resizeLinearDownscale(const AImage& src, uint32_t width, uint32_t height);
    static void copy(const AImage& src, AImage& dst, uint32_t x, uint32_t y);

    [[nodiscard]]
    uint8_t& at(uint32_t x, uint32_t y) {
        return mData[(y * getWidth() + x) * getBytesPerPixel()];
    }

    [[nodiscard]]
    const uint8_t& at(uint32_t x, uint32_t y) const {
        return mData[(y * getWidth() + x) * getBytesPerPixel()];
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

