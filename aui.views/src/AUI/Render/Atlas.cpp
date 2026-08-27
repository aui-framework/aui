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

namespace aui {
    Atlas::Atlas(IRendererBackend& renderer, APixelFormat format, glm::u32vec2 pageSize, TextureFilter filter) :
        mRenderer(renderer), mFormat(format), mPageSize(pageSize), mFilter(filter) {}

    Atlas::Page::Page(IRendererBackend& renderer, APixelFormat format, glm::u32vec2 pageSize, TextureFilter filter) :
        texture(renderer.createTexture(pageSize, format, filter)),
        image({ pageSize.x, pageSize.y }, format),
        packer(pageSize.x, pageSize.y) {
    }

    Atlas::Handle Atlas::insert(AImageView image) {
        Rect r;
        Page* targetPage = nullptr;
        for (auto& page : mPages) {
            if (page->packer.allocateRect(r, image.width(), image.height())) {
                targetPage = page.get();
                break;
            }
        }
        if (targetPage == nullptr) {
            auto& page = mPages.emplace_back(std::make_unique<Page>(mRenderer, mFormat, mPageSize, mFilter));
            if (!page->packer.allocateRect(r, image.width(), image.height())) {
                return { {}, nullptr };
            }
            targetPage = page.get();
        }
    targetPage->image.insert({r.x, r.y}, image);

    const float BIAS = 0.1f;
    glm::vec4 uv(r.x, r.y, r.x + r.width, r.y + r.height);
    uv.x += BIAS; uv.y += BIAS; uv.z -= BIAS; uv.w -= BIAS;

    // Normalize UV
    uv /= glm::vec4(mPageSize.x, mPageSize.y, mPageSize.x, mPageSize.y);

    targetPage->isTextureInvalid = true;
    return { uv, targetPage->texture };
}

void Atlas::syncWithGpu() {
    for (auto& page : mPages) {
        if (page->isTextureInvalid) {
            page->texture->upload(page->image);
            page->isTextureInvalid = false;
        }
    }
}
}
