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

#include "Atlas.hpp"

#include <AUI/Render/IRendererBackend.h>
#include <AUI/Render/ITexture.h>
#include <AUI/Image/AImage.h>

namespace aui {
    Atlas::Atlas(IRendererBackend& renderer, APixelFormat format, glm::u32vec2 pageSize) :
        mRenderer(renderer), mFormat(format), mPageSize(pageSize) {}

    Atlas::Page::Page(IRendererBackend& renderer, APixelFormat format, glm::u32vec2 pageSize) :
        texture(renderer.createTexture(pageSize, format)) {
        AImage dummy({ 1, 1 }, format);
        texturePacker.resize(dummy, pageSize.x);
    }

    Atlas::Handle Atlas::insert(AImageView image) {
        glm::vec4 uv;
        Page* targetPage = nullptr;
        for (auto& page : mPages) {
            if (page->texturePacker.tryInsert(image, uv)) {
                targetPage = page.get();
                break;
            }
        }
        if (targetPage == nullptr) {
            auto& page = mPages.emplace_back(std::make_unique<Page>(mRenderer, mFormat, mPageSize));
            if (!page->texturePacker.tryInsert(image, uv)) {
                return { {}, nullptr };
            }
            targetPage = page.get();
        }

        const float BIAS = 0.1f;
        uv.x += BIAS; uv.y += BIAS; uv.z -= BIAS; uv.w -= BIAS;

        // Normalize UV
        uv /= glm::vec4(mPageSize.x, mPageSize.y, mPageSize.x, mPageSize.y);

        targetPage->isTextureInvalid = true;
        return { uv, targetPage->texture };
    }

    void Atlas::syncWithGpu() {
        for (auto& page : mPages) {
            if (page->isTextureInvalid) {
                if (auto img = page->texturePacker.getImage()) {
                    page->texture->upload(*img);
                }
                page->isTextureInvalid = false;
            }
        }
    }
}
