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

#include <AUI/Image/AImage.h>


/**
 * @brief Texture filtering mode.
 */
enum class TextureFilter {
    NEAREST,
    LINEAR,
};

enum class TextureOrigin {
    TOP_LEFT,
    BOTTOM_LEFT,
};

/**
 * @brief Renderer-friendly image representation.
 */
class ITexture {
public:
    /**
     * @brief Copies image to the texture image representation (i.e. to GPU memory).
     */
    virtual void upload(AImageView image) = 0;

    [[nodiscard]]
    virtual glm::u32vec2 getSize() const = 0;

    [[nodiscard]]
    virtual APixelFormat getFormat() const = 0;

    [[nodiscard]]
    TextureOrigin getOrigin() const { return mOrigin; }
    void setOrigin(TextureOrigin origin) { mOrigin = origin; }

    virtual ~ITexture() = default;

protected:
    TextureOrigin mOrigin = TextureOrigin::TOP_LEFT;
};
