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

#include <glm/glm.hpp>
#include <AUI/Common/AVector.h>
#include <AUI/Common/ADeque.h>
#include <AUI/Common/SharedPtr.h>
#include <AUI/Image/APixelFormat.h>
#include <AUI/Image/AImage.h>
#include <AUI/Image/AImageView.h>
#include <AUI/Render/RectPacker.hpp>
#include <AUI/Render/ITexture.h>

class ITexture;
class IRendererBackend;

namespace aui {
/**
 * @brief Generic paged texture atlas.
 */
class API_AUI_VIEWS Atlas: public aui::noncopyable {
    struct Page {
        _<ITexture> texture;
        AImage image;
        RectPacker packer;
        bool isTextureInvalid = true;

        Page(IRendererBackend& renderer, APixelFormat format, glm::u32vec2 pageSize, TextureFilter filter);
    };

    IRendererBackend& mRenderer;
    APixelFormat mFormat;
    glm::u32vec2 mPageSize;
    TextureFilter mFilter;
    ADeque<_unique<Page>> mPages;

public:
    struct Handle {
        glm::vec4 uv;
        _<ITexture> texture;
    };

    Atlas(IRendererBackend& renderer, APixelFormat format, glm::u32vec2 pageSize = { 1024, 1024 }, TextureFilter filter = TextureFilter::LINEAR);

    Handle insert(AImageView image);

    /**
     * @brief Synchronizes modified pages with the GPU.
     */
    void syncWithGpu();
};
}
