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
#include "AUI/Traits/memory.h"
#include "AImageFormat.h"


class API_AUI_IMAGE AImage;
class API_AUI_IMAGE AImageView;

template<auto imageFormat = AImageFormat::DEFAULT>
class AFormattedImageView;

template<auto imageFormat = AImageFormat::DEFAULT>
class AFormattedImage;

template<typename T>
concept AImageViewVisitor = requires(T t) {
    t(std::declval<const AFormattedImageView<AImageFormat::RGB  | AImageFormat::BYTE>&>());
    t(std::declval<const AFormattedImageView<AImageFormat::RGBA | AImageFormat::BYTE>&>());
    t(std::declval<const AFormattedImageView<AImageFormat::RGB  | AImageFormat::FLOAT>&>());
    t(std::declval<const AFormattedImageView<AImageFormat::RGBA | AImageFormat::FLOAT>&>());
    // etc...
};

template<typename T>
concept AImageVisitor = requires(T t) {
    t(std::declval<AFormattedImage<AImageFormat::RGB  | AImageFormat::BYTE>&>());
    t(std::declval<AFormattedImage<AImageFormat::RGBA | AImageFormat::BYTE>&>());
    t(std::declval<AFormattedImage<AImageFormat::RGB  | AImageFormat::FLOAT>&>());
    t(std::declval<AFormattedImage<AImageFormat::RGBA | AImageFormat::FLOAT>&>());
    // etc...
};

/**
 * @brief Non-owning read-only image representation of some format.
 * @ingroup image
 * @details
 */
class API_AUI_IMAGE AImageView {
public:
    using Color = AColor;

    AImageView(): mSize(0, 0) {}
    AImageView(AByteBufferView data, glm::uvec2 size, AImageFormat format) : mData(data), mSize(size), mFormat(format) {}
    AImageView(const AImage& v);

    /**
     * @return Raw image pixel data.
     */
    [[nodiscard]]
    AByteBufferView buffer() const noexcept {
        return mData;
    }

    /**
     * @return Image size.
     */
    [[nodiscard]]
    glm::uvec2 size() const noexcept {
        return mSize;
    }

    /**
     * @return Image width.
     */
    [[nodiscard]]
    unsigned width() const noexcept {
        return mSize.x;
    }

    /**
     * @return Image height.
     */
    [[nodiscard]]
    unsigned height() const noexcept {
        return mSize.y;
    }

    /**
     * @return Image pixel format.
     */
    [[nodiscard]]
    AImageFormat format() const noexcept {
        return mFormat;
    }

    /**
     * @return Bytes per pixel.
     */
    [[nodiscard]]
    std::uint8_t bytesPerPixel() const noexcept {
        return mFormat.bytesPerPixel();
    }

    /**
     * @brief Retrieves pixel color data.
     * @param position position
     * @return color of specified pixel
     * @details
     * This is a universal function that basically allows you to don't care about underlying image format
     * representation. For performance critical code you may want to use visit method.
     *
     * @note
     * Specifying position out of image range causes assertion fail in debug or undefined behaviour.
     */
    [[nodiscard]]
    Color get(glm::uvec2 position) const noexcept;

    template<AImageViewVisitor Visitor>
    auto visit(Visitor&& visitor) const;


    /**
     * @brief Crops the image, creating new image with the same format.
     * @param position offset
     * @param size size of new image
     */
    [[nodiscard]]
    AImage cropped(glm::uvec2 position, glm::uvec2 size) const;


    [[nodiscard]]
    Color averageColor() const noexcept;

    /**
     * @brief Retrieves reference to raw data at specified position.
     * @param position position
     * @return reference to raw pixel data
     * @note
     * Specifying position out of image range causes assertion fail in debug or undefined behaviour.
     */
    [[nodiscard]]
    const char& rawDataAt(glm::uvec2 position) const noexcept {
        assert(position.x < width());
        assert(position.y < height());
        return mData.at<char>((position.y * width() + position.x) * bytesPerPixel());
    }

    [[nodiscard]]
    AImage mirroredVertically() const;

    [[nodiscard]]
    AImage resizedLinearDownscale(glm::uvec2 newSize) const;

    [[nodiscard]]
    AImage convert(AImageFormat format) const;


    /**
     * @brief Shortcut to buffer().data().
     */
    [[nodiscard]]
    const char* data() const noexcept {
        return buffer().data();
    }

protected:
    AByteBufferView mData;
    glm::uvec2 mSize;
    AImageFormat mFormat = AImageFormat::UNKNOWN;
};

/**
 * @brief Same as AImageView but all universal AColor methods replaced with concrete specific AFormattedColor type thus
 * can be used by performance critical code.
 */
template<auto f>
class AFormattedImageView: public AImageView {
public:
    using Color = AFormattedColor<f>;
    static constexpr int FORMAT = f;

    constexpr int format() const noexcept {
        return f;
    }

    AFormattedImageView() {
        mFormat = f;
    }

    AFormattedImageView(AByteBufferView data, glm::uvec2 size) : AImageView(data, size, f) {}

    [[nodiscard]]
    const Color& get(glm::uvec2 position) const noexcept {
        return reinterpret_cast<const Color&>(rawDataAt(position));
    }

    [[nodiscard]]
    const Color* begin() const noexcept {
        return reinterpret_cast<const Color*>(AImageView::data());
    }

    [[nodiscard]]
    const Color* end() const noexcept {
        return reinterpret_cast<const Color*>(AImageView::data()) + (width() * height());
    }

    [[nodiscard]]
    AColor averageColor() const noexcept {
        AColor accumulator;
        aui::zero(accumulator);

        for (auto i = begin(); i != end(); ++i) {
            accumulator += AColor(*i);
        }

        return accumulator / float(width() * height());
    }
};


template<AImageViewVisitor Visitor>
auto AImageView::visit(Visitor&& visitor) const {
#define AUI_CASE(v) case v: { \
    switch(format() & AImageFormat::TYPE_BITS) { \
    case AImageFormat::BYTE: return visitor(reinterpret_cast<const AFormattedImageView<v | AImageFormat::BYTE>&>(*this)); \
    case AImageFormat::FLOAT: return visitor(reinterpret_cast<const AFormattedImageView<v | AImageFormat::FLOAT>&>(*this)); \
                               }}

    switch (format() & AImageFormat::COMPONENT_BITS) {
        AUI_CASE(AImageFormat::R)
        AUI_CASE(AImageFormat::RG)
        AUI_CASE(AImageFormat::RGB)
        AUI_CASE(AImageFormat::RGBA)
        AUI_CASE(AImageFormat::ARGB)
        AUI_CASE(AImageFormat::BGRA)

        default:
            throw AException("unknown color format");
    }
#undef AUI_CASE
}

/**
 * @brief Owning image representation.
 * @ingroup image
 * @details
 */
class API_AUI_IMAGE AImage: public AImageView {
private:
    class Cache;
    friend class ::Cache<AImage, Cache, AUrl>;
    friend class AImageLoaderRegistry;
    friend class AImageView;
    class Cache: public ::Cache<AImage, Cache, AUrl> {
    public:
        static Cache& inst();
    protected:
        _<AImage> load(const AUrl& key) override;
    };

    void setPixelAt(std::uint32_t index, glm::ivec4 color);

public:
    AImage() = default;

    explicit AImage(AImageView imageView): AImage(imageView.buffer(), imageView.size(), imageView.format()) {}

    AImage(AByteBuffer buffer, glm::uvec2 size, AImageFormat format): mOwnedBuffer(std::move(buffer)) {
        mSize = size;
        mFormat = format;
        mData = mOwnedBuffer;
    }

    AImage(glm::uvec2 size, AImageFormat format) {
        mSize = size;
        mFormat = format;
        allocate();
    }

    [[nodiscard]]
    AByteBuffer& modifiableBuffer() noexcept {
        return mOwnedBuffer;
    }


    template<AImageVisitor Visitor>
    auto visit(Visitor&& visitor);

    void set(glm::uvec2 position, Color c) noexcept;

    void insert(glm::uvec2 position, AImageView image);


    void mirrorVertically();

    [[nodiscard]]
    static _<AImage> fromUrl(const AUrl& url);

    [[nodiscard]]
    static _<AImage> fromFile(const APath& path);

    [[nodiscard]]
    static _<AImage> fromBuffer(AByteBufferView buffer);


private:
    AByteBuffer mOwnedBuffer;


    void allocate() {
        mOwnedBuffer.resize(width() * height() * bytesPerPixel());
        mData = mOwnedBuffer;
    }
};


/**
 * @brief Same as AImage but all universal AColor methods replaced with concrete specific AFormattedColor type thus
 * can be used by performance critical code.
 */
template<auto f>
class AFormattedImage: public AImage {
public:
    using Color = AFormattedColor<f>;
    static constexpr auto FORMAT = f;


    AFormattedImage() {
        mFormat = f;
    }

    AFormattedImage(AByteBuffer data, glm::uvec2 size) : AImage(std::move(data), size, f) {}
    AFormattedImage(glm::uvec2 size) : AImage(size, f) {}


    constexpr int format() const noexcept {
        return f;
    }

    [[nodiscard]]
    const Color& get(glm::uvec2 position) const noexcept {
        return reinterpret_cast<const Color&>(rawDataAt(position));
    }

    void set(glm::uvec2 position, Color color) noexcept {
        const_cast<Color&>(get(position)) = color;
    }

    [[nodiscard]]
    const Color* begin() const noexcept {
        return reinterpret_cast<const Color*>(AImageView::data());
    }

    [[nodiscard]]
    const Color* end() const noexcept {
        return reinterpret_cast<const Color*>(AImageView::data()) + (width() * height());
    }

    [[nodiscard]]
    Color* begin() noexcept {
        return reinterpret_cast<Color*>(AImage::modifiableBuffer().begin());
    }

    [[nodiscard]]
    Color* end() noexcept {
        return reinterpret_cast<Color*>(AImage::modifiableBuffer().end());
    }

    [[nodiscard]]
    Color averageColor() const noexcept {
        Color accumulator;
        aui::zero(accumulator);

        for (auto i = begin(); i != end(); ++i) {
            accumulator += *i;
        }

        return accumulator / (width() * height());
    }
};


template<AImageVisitor Visitor>
auto AImage::visit(Visitor&& visitor) {
    return AImageView::visit([&](const auto& image) {
        static constexpr int format = std::decay_t<decltype(image)>::FORMAT;
        return visitor(const_cast<AFormattedImage<format>&>(reinterpret_cast<const AFormattedImage<format>&>(image)));
    });
}