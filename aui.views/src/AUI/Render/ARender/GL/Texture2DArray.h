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

#include "AUI/Common/SharedPtr.h"
#include "AUI/Image/AImage.h"
#include "Texture.h"
#include "GLEnums.h"

namespace gl {

/**
 * @brief Represents a 2D texture array.
 * @details
 * A 2D texture array is a texture that contains multiple 2D textures, each accessible by its index. It's useful for
 * storing and accessing multiple textures with the same dimensions.
 *
 * After creating a 2D texture array, you can use `update` to update a single texture in the array. You cannot update
 * dimensions of the texture array, so you should create a new one if you want to change the size of the texture array.
 */
class API_AUI_VIEWS Texture2DArray final: public Texture<gl::TEXTURE_2D_ARRAY> {
public:
    /**
     * @brief Initializes the 2D texture array with empty data.
     * @param textureSize The size of the 2D texture (width and height).
     * @param textureCount The number of layers in the texture array.
     *
     * @details
     * This function allocates storage for a 2D texture array with the specified size and texture count.
     * It binds the texture and uses `glTexStorage3D` to allocate the storage.
     */
    Texture2DArray(glm::uvec2 textureSize, unsigned textureCount);

    ~Texture2DArray() override = default;

    /**
     * @brief Updates a single texture of the 2D texture array.
     * @param texture The index of the texture to be updated.
     * @param image The image to be used as the new texture.
     *
     * @details
     * This function binds the texture and uploads the provided image as a texture in the 2D texture array.
     * It uses `glTexSubImage3D` to update the texture.
     *
     * This function expects the image to have the same size as the texture size initialized earlier, otherwise, an
     * assertion error is thrown.
     */
    void update(unsigned texture, AImageView image);

    Texture2DArray(Texture2DArray&&) noexcept = default;
    Texture2DArray& operator=(Texture2DArray&&) noexcept = default;

private:
    glm::uvec2 mTextureSize{};
    unsigned mTextureCount{};
};
}   // namespace gl
