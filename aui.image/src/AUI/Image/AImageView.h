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
#include "APixelFormat.h"
#include "glm/vector_relational.hpp"

class API_AUI_IMAGE AImage;
class API_AUI_IMAGE AImageView;

template <auto imageFormat = APixelFormat::R8G8B8A8_UNORM>
class AFormattedImageView;

template <auto imageFormat = APixelFormat::R8G8B8A8_UNORM>
class AFormattedImage;

template <typename T>
concept AImageViewVisitor = requires(T t) {
    t(std::declval<const AFormattedImageView<APixelFormat::R8_UNORM>&>());
    t(std::declval<const AFormattedImageView<APixelFormat::R8G8_UNORM>&>());
    t(std::declval<const AFormattedImageView<APixelFormat::R8G8B8_UNORM>&>());
    t(std::declval<const AFormattedImageView<APixelFormat::R8G8B8A8_UNORM>&>());
    t(std::declval<const AFormattedImageView<APixelFormat::B8G8R8A8_UNORM>&>());
    t(std::declval<const AFormattedImageView<APixelFormat::R16G16B16A16_SFLOAT>&>());
    t(std::declval<const AFormattedImageView<APixelFormat::R32G32B32A32_SFLOAT>&>());
    t(std::declval<const AFormattedImageView<APixelFormat::A2R10G10B10_UNORM_PACK32>&>());
};

template <typename T>
concept AImageVisitor = requires(T t) {
    t(std::declval<AFormattedImage<APixelFormat::R8_UNORM>&>());
    t(std::declval<AFormattedImage<APixelFormat::R8G8_UNORM>&>());
    t(std::declval<AFormattedImage<APixelFormat::R8G8B8_UNORM>&>());
    t(std::declval<AFormattedImage<APixelFormat::R8G8B8A8_UNORM>&>());
    t(std::declval<AFormattedImage<APixelFormat::B8G8R8A8_UNORM>&>());
    t(std::declval<AFormattedImage<APixelFormat::R16G16B16A16_SFLOAT>&>());
    t(std::declval<AFormattedImage<APixelFormat::R32G32B32A32_SFLOAT>&>());
    t(std::declval<AFormattedImage<APixelFormat::A2R10G10B10_UNORM_PACK32>&>());
};

/**
 * @brief Non-owning read-only image representation of some format.
 * @ingroup image
 * @details
 */
class API_AUI_IMAGE AImageView {
public:
    using Color = AColor;

    AImageView() : mSize(0, 0), mStride(0) {}

    /**
     * @brief Constructs an image view with an explicit row stride.
     *
     * This constructor creates a non-owning view over image data using the
     * provided byte buffer and metadata. The stride (bytes per row)
     * is explicitly specified, which allows the view to work with padded or
     * externally aligned image memory.
     *
     * @param data   Non-owning byte buffer that contains the image pixels.
     * @param stride Number of bytes between the start of consecutive rows.
     * @param size   Image dimensions in pixels (width, height).
     * @param format Pixel format describing how to interpret each pixel.
     */
    AImageView(AByteBufferView data, size_t stride, glm::uvec2 size, APixelFormat format)
        : mData(data), mStride(stride), mSize(size), mFormat(format) {}

    /**
     * @brief Constructs an image view with tightly packed rows.
     *
     * This constructor assumes the image rows are tightly packed in memory
     * (no padding between rows). The stride is automatically computed as:
     *
     *     stride = bytesPerPixel() * width()
     *
     * Use this overload only when the source image buffer has no row padding.
     *
     * @param data   Non-owning byte buffer that contains the image pixels.
     * @param size   Image dimensions in pixels (width, height).
     * @param format Pixel format describing how to interpret each pixel.
     */
    AImageView(AByteBufferView data, glm::uvec2 size, APixelFormat format)
        : mData(data), mSize(size), mFormat(format) {
        mStride = bytesPerPixel() * width();
    }

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
    APixelFormat format() const noexcept {
        return mFormat;
    }

    /**
     * @return Bytes per pixel.
     */
    [[nodiscard]]
    std::uint8_t bytesPerPixel() const noexcept {
        return ::bytesPerPixel(mFormat);
    }

    /**
     * @brief Retrieves pixel color data.
     * @param position position
     * @return color of specified pixel
     * @details
     * This is a universal function that basically allows you to don't care about underlying image format
     * representation. For performance critical code you may want to use visit method.
     *
     * Specifying position out of image range causes assertion fail in debug or undefined behaviour.
     */
    [[nodiscard]]
    Color get(glm::uvec2 position) const noexcept;

    template <AImageViewVisitor Visitor>
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
     * @details
     * Specifying position out of image range causes assertion fail in debug or undefined behaviour.
     */
    [[nodiscard]]
    const char& rawDataAt(glm::uvec2 position) const noexcept {
        AUI_ASSERT(width() != 0);
        AUI_ASSERT(height() != 0);
        AUI_ASSERT(mStride != 0);
        AUI_ASSERT(position.x < width());
        AUI_ASSERT(position.y < height());
        size_t index = (position.y * mStride) + (position.x * bytesPerPixel());
        AUI_ASSERT(index < mData.size());
        return mData.at<char>(index);
    }

    [[nodiscard]]
    AImage mirroredVertically() const;

    [[nodiscard]]
    AImage resizedLinearDownscale(glm::uvec2 newSize) const;

    /**
     * @brief Converts (if needed) the image to the format `desiredFormat` known at compile time. The image is then
     * passed to `consumer`.
     * @tparam desiredFormat the image format to convert to
     * @param consumer the consumer function that will accept the image view of format `desiredFormat`
     * @details
     * Guarantees that the image passed into consumer is in the pixel format `desiredFormat`.
     *
     * ## Two possible paths
     * ### 1) The image is already in desiredFormat
     *
     * Then the function avoids any work and passes the existing image view directly to consumer.
     *
     * - No conversion
     * - No allocation
     * - Minimal overhead
     * - This is the fast path.
     *
     * ### 2) The image is in a different format
     *
     * Then the function:
     *
     * - creates an owning image object,
     * - converts the pixels into `desiredFormat`,
     * - passes the converted image view to `consumer`.
     *
     * So the callback always receives the format it asked for.
     *
     * ## Why it exists
     *
     * This is useful when you want code that only works with one pixel format, but the input may come in several
     * formats. For example, you can write an image saver that expects RGBA and let convert handle the conversion if
     * needed.
     *
     * The function is a template on the target format, so the desired pixel format is known at compile time. That lets
     * the API be type-safe and efficient.
     *
     * ## In short
     * “Give me an image in format X, and I’ll call your callback with an image view in format X, converting only if
     * necessary.”
     */
    template<auto /* APixelFormat */ desiredFormat>
    void convert(aui::invocable<AFormattedImageView<desiredFormat>> auto && consumer) const;

    /**
     * @brief Converts the image to the format `desiredFormat` known at runtime.
     * @param desiredFormat the image format to convert to
     * @return Owning, type-erased representation of an image in `desiredFormat`
     * @details
     * In contrast to the template version of `convert`, this overload is easier to use at the cost of runtime overhead.
     */
    [[nodiscard]]
    AImage convert(APixelFormat desiredFormat) const;


    /**
     * @brief Shortcut to buffer().data().
     */
    [[nodiscard]]
    const char* data() const noexcept {
        return buffer().data();
    }

    [[nodiscard]]
    size_t stride() const noexcept {
        return mStride;
    }

protected:
    AByteBufferView mData;
    size_t mStride;
    glm::uvec2 mSize;
    APixelFormat mFormat = APixelFormat::UNKNOWN;
};

/**
 * @brief Same as AImageView but all universal AColor methods replaced with concrete specific AFormattedColor type thus
 * can be used by performance critical code.
 */
template <auto /* APixelFormat */ f>
class AFormattedImageView : public AImageView {
public:
    using Color = AFormattedColor<f>;
    static constexpr APixelFormat FORMAT = f;

    constexpr APixelFormat format() const noexcept { return f; }

    AFormattedImageView() { mFormat = f; }

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

template <AImageViewVisitor Visitor>
auto AImageView::visit(Visitor&& visitor) const {
    switch (format()) {
        case APixelFormat::R8_UNORM:
            return visitor(reinterpret_cast<const AFormattedImageView<APixelFormat::R8_UNORM>&>(*this));
        case APixelFormat::R8G8_UNORM:
            return visitor(reinterpret_cast<const AFormattedImageView<APixelFormat::R8G8_UNORM>&>(*this));
        case APixelFormat::R8G8B8_UNORM:
            return visitor(reinterpret_cast<const AFormattedImageView<APixelFormat::R8G8B8_UNORM>&>(*this));
        case APixelFormat::R8G8B8A8_UNORM:
            return visitor(reinterpret_cast<const AFormattedImageView<APixelFormat::R8G8B8A8_UNORM>&>(*this));
        case APixelFormat::B8G8R8A8_UNORM:
            return visitor(reinterpret_cast<const AFormattedImageView<APixelFormat::B8G8R8A8_UNORM>&>(*this));
        case APixelFormat::R16G16B16A16_SFLOAT:
            return visitor(reinterpret_cast<const AFormattedImageView<APixelFormat::R16G16B16A16_SFLOAT>&>(*this));
        case APixelFormat::R32G32B32A32_SFLOAT:
            return visitor(reinterpret_cast<const AFormattedImageView<APixelFormat::R32G32B32A32_SFLOAT>&>(*this));
        case APixelFormat::A2R10G10B10_UNORM_PACK32:
            return visitor(reinterpret_cast<const AFormattedImageView<APixelFormat::A2R10G10B10_UNORM_PACK32>&>(*this));
        default:
            throw AException("unknown color format");
    }
}
