/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

//
// Created by alex2772 on 25.07.2018.
//

#pragma once

#include <AUI/Image/AImageView.h>

/**
 * @brief Owning image representation.
 * @ingroup image
 * @details
 */
class API_AUI_IMAGE AImage : public AImageView {
private:
    class Cache;
    friend class ::Cache<AImage, Cache, AUrl>;
    friend class AImageLoaderRegistry;
    friend class AImageView;

    void setPixelAt(std::uint32_t index, glm::ivec4 color);

public:
    AImage() = default;

    explicit AImage(AImageView imageView) : AImage(imageView.buffer(), imageView.size(), imageView.format()) {}

    AImage(AByteBuffer buffer, glm::uvec2 size, APixelFormat format) : mOwnedBuffer(std::move(buffer)) {
        mSize = size;
        mFormat = format;
        mData = mOwnedBuffer;
    }

    AImage(glm::uvec2 size, APixelFormat format) {
        mSize = size;
        mFormat = format;
        allocate();
    }

    [[nodiscard]]
    AByteBuffer& modifiableBuffer() noexcept {
        return mOwnedBuffer;
    }

    template <AImageVisitor Visitor>
    auto visit(Visitor&& visitor);

    void set(glm::uvec2 position, Color c) noexcept;

    void insert(glm::uvec2 position, AImageView image);

    void fill(Color color);

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
template <auto f>
class AFormattedImage : public AImage {
public:
    using Color = AFormattedColor<f>;
    static constexpr auto FORMAT = f;

    AFormattedImage() { mFormat = f; }

    AFormattedImage(AByteBuffer data, glm::uvec2 size) : AImage(std::move(data), size, f) {}
    AFormattedImage(glm::uvec2 size) : AImage(size, f) {}

    constexpr int format() const noexcept { return f; }

    [[nodiscard]]
    const Color& get(glm::uvec2 position) const noexcept {
        return reinterpret_cast<const Color&>(rawDataAt(position));
    }

    void set(glm::uvec2 position, Color color) noexcept { const_cast<Color&>(get(position)) = color; }

    void setWithPositionCheck(glm::uvec2 position, Color color) noexcept {
        if (glm::any(glm::greaterThanEqual(position, size()))) {
            return;
        }
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

    void fill(Color color) { std::fill(begin(), end(), color); }

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

template <AImageVisitor Visitor>
auto AImage::visit(Visitor&& visitor) {
    return AImageView::visit([&](const auto& image) {
        static constexpr int format = std::decay_t<decltype(image)>::FORMAT;
        return visitor(const_cast<AFormattedImage<format>&>(reinterpret_cast<const AFormattedImage<format>&>(image)));
    });
}