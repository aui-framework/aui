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

namespace gl {
constexpr uint32_t TEXTURE_1D = 0x0DE1;
constexpr uint32_t TEXTURE_2D = 0x0DE1;
constexpr uint32_t TEXTURE_3D = 0x806F;
constexpr uint32_t TEXTURE_2D_ARRAY = 0x8C1A;

template <unsigned TEXTURE_TARGET>
class API_AUI_VIEWS Texture : public aui::noncopyable {
public:
    Texture();
    virtual ~Texture();
    void setupNearest();
    void setupLinear();
    void setupClampToEdge();
    void setupRepeat();
    void setupMirroredRepeat();

    Texture(Texture&& rhs) noexcept
      : mTexture(std::exchange(rhs.mTexture, 0)), mFiltering(rhs.mFiltering), mWrapping(rhs.mWrapping) {}

    Texture& operator=(Texture&& rhs) noexcept {
        mTexture = std::exchange(rhs.mTexture, 0);
        mFiltering = std::exchange(rhs.mFiltering, Filtering::UNDEFINED);
        mWrapping = std::exchange(rhs.mWrapping, Wrapping::UNDEFINED);
        return *this;
    }

    void bind(uint8_t index = 0);
    static void unbind(uint8_t index = 0);

    operator bool() const { return mTexture; }
    uint32_t getHandle() const { return mTexture; }

private:
    uint32_t mTexture = 0;
    enum class Filtering {
        UNDEFINED,
        NEAREST,
        LINEAR,
    } mFiltering = Filtering::UNDEFINED;

    enum class Wrapping {
        UNDEFINED,
        CLAMP_TO_EDGE,
        REPEAT,
        MIRRORED_REPEAT,
    } mWrapping = Wrapping::UNDEFINED;
};
}   // namespace gl