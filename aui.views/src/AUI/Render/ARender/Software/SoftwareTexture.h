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

#include <AUI/Render/ITexture.h>

class SoftwareTexture: public ITexture {
protected:
    AImage mImage;
    TextureFilter mFilter = TextureFilter::LINEAR;

public:
    void upload(AImageView image) override;
    void upload(AImage&& image) {
        mImage = std::move(image);
    }

    [[nodiscard]]
    glm::u32vec2 getSize() const override {
        return mImage.size();
    }

    [[nodiscard]]
    APixelFormat getFormat() const override {
        return mImage.format();
    }

    [[nodiscard]] const AImage& getImage() const noexcept {
        return mImage;
    }

    [[nodiscard]] AImage& getImage() noexcept {
        return mImage;
    }

    void setFilter(TextureFilter filter) {
        mFilter = filter;
    }

    [[nodiscard]] TextureFilter getFilter() const noexcept {
        return mFilter;
    }
};
