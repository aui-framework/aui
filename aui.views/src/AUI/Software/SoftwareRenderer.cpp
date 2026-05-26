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

#include "SoftwareRenderer.h"
#include <AUI/Render/IRenderer.h>
#include <AUI/Traits/callables.h>
#include <AUI/Platform/ASurface.h>
#include <AUI/ASS/Property/Backdrop.h>
#include <AUI/Traits/values.h>
#include <AUI/Render/ACanvas.hpp>
#include <AUI/Image/AImage.h>
#include "SoftwareTexture.h"

SoftwareRenderer::SoftwareRenderer() {}

void SoftwareRenderer::setWindow(ASurface* window) {
    mWindow = window;
    mContext = dynamic_cast<SoftwareRenderingContext*>(window->getRenderingContext().get());
}

void SoftwareRenderer::putPixel(glm::ivec2 pos, AColor color, Blending blending) {
    if (!mContext) return;
    if (pos.x < 0 || pos.y < 0 || (uint32_t)pos.x >= mContext->bitmapSize().x || (uint32_t)pos.y >= mContext->bitmapSize().y) return;
    
    if (color.a >= 0.999f || blending == Blending::INVERSE_SRC) {
        mContext->putPixel(glm::uvec2(pos), glm::u8vec4(color * 255.f));
    } else {
        auto dst = glm::vec4(mContext->getPixel(glm::uvec2(pos))) / 255.f;
        auto combined = dst * (1.f - color.a) + color * color.a;
        mContext->putPixel(glm::uvec2(pos), glm::u8vec4(combined * 255.f));
    }
}

void SoftwareRenderer::solidRectangles(const ADisplayList::SolidRectangles& v, const glm::mat4& transform, Blending blending) {
    for (const auto& inst : v.instances) {
        auto p1 = transform * glm::vec4(inst.position, 0.f, 1.f);
        auto p2 = transform * glm::vec4(inst.position + inst.size, 0.f, 1.f);
        for (int y = (int)p1.y; y < (int)p2.y; ++y) {
            for (int x = (int)p1.x; x < (int)p2.x; ++x) {
                putPixel({x, y}, inst.color, blending);
            }
        }
    }
}
void SoftwareRenderer::gradientRectangles(const ADisplayList::GradientRectangles& v, const glm::mat4& transform, Blending blending) {}
void SoftwareRenderer::texturedRectangles(const ADisplayList::TexturedRectangles& v, const glm::mat4& transform, Blending blending) {
    auto texture = dynamic_cast<SoftwareTexture*>(v.texture.get());
    if (!texture) return;
    const auto& img = texture->getImage();
    if (img.width() == 0 || img.height() == 0) return;

    for (const auto& inst : v.instances) {
        auto p1 = transform * glm::vec4(inst.position, 0.f, 1.f);
        auto p2 = transform * glm::vec4(inst.position + inst.size, 0.f, 1.f);
        
        float width = p2.x - p1.x;
        float height = p2.y - p1.y;

        for (int y = (int)p1.y; y < (int)p2.y; ++y) {
            for (int x = (int)p1.x; x < (int)p2.x; ++x) {
                float u = (x - (float)p1.x) / width;
                float v_uv = (y - (float)p1.y) / height;
                
                glm::uvec2 texPos((unsigned)(u * glm::max(0.f, (float)img.width() - 1.f)), (unsigned)(v_uv * glm::max(0.f, (float)img.height() - 1.f)));
                AColor texColor;
                if (texPos.x < img.width() && texPos.y < img.height()) {
                    texColor = img.get(texPos);
                } else {
                    texColor = AColor::RED;
                }
                putPixel({x, y}, texColor * inst.color, blending);
            }
        }
    }
}
void SoftwareRenderer::solidRoundedRectangles(const ADisplayList::SolidRoundedRectangles& v, const glm::mat4& transform, Blending blending) {}
void SoftwareRenderer::gradientRoundedRectangles(const ADisplayList::GradientRoundedRectangles& v, const glm::mat4& transform, Blending blending) {}
void SoftwareRenderer::texturedRoundedRectangles(const ADisplayList::TexturedRoundedRectangles& v, const glm::mat4& transform, Blending blending) {}
void SoftwareRenderer::rectangleBorders(const ADisplayList::RectangleBorders& v, const glm::mat4& transform, Blending blending) {}
void SoftwareRenderer::roundedRectangleBorders(const ADisplayList::RoundedRectangleBorders& v, const glm::mat4& transform, Blending blending) {}
void SoftwareRenderer::boxShadow(const ADisplayList::BoxShadow& v, const glm::mat4& transform, Blending blending) {}
void SoftwareRenderer::boxShadowInner(const ADisplayList::BoxShadowInner& v, const glm::mat4& transform, Blending blending) {}
void SoftwareRenderer::glyphs(const ADisplayList::Glyphs& v, const glm::mat4& transform, Blending blending) {
    auto texture = dynamic_cast<SoftwareTexture*>(v.texture.get());
    if (!texture)
        return;
    const auto& img = texture->getImage();
    if (img.width() == 0)
        return;

    for (const auto& inst : v.instances) {
        auto p1 = transform * glm::vec4(inst.position, 0.f, 1.f);
        auto p2 = transform * glm::vec4(inst.position + inst.size, 0.f, 1.f);

        float width = p2.x - p1.x;
        float height = p2.y - p1.y;

        for (int y = (int) p1.y; y < (int) p2.y; ++y) {
            for (int x = (int) p1.x; x < (int) p2.x; ++x) {
                float u = (x - (float) p1.x) / width;
                float v_uv = (y - (float) p1.y) / height;

                float tx = glm::mix(inst.u1.x, inst.u2.x, u) * (float)img.width();
                float ty = glm::mix(inst.u1.y, inst.u2.y, v_uv) * (float)img.height();

                glm::uvec2 texPos((unsigned)tx, (unsigned)ty);
                if (texPos.x >= img.width() || texPos.y >= img.height()) continue;

                AColor maskColor = img.get(texPos);
                AColor finalColor = inst.color * v.color;

                if (!v.isSubpixel) {
                    finalColor.a *= maskColor.r;
                    putPixel({x, y}, finalColor, blending);
                } else {
                    if (!mContext) continue;
                    if (x < 0 || y < 0 || (uint32_t)x >= mContext->bitmapSize().x || (uint32_t)y >= mContext->bitmapSize().y) continue;
                    
                    auto dst = glm::vec4(mContext->getPixel(glm::uvec2(x, y))) / 255.f;
                    
                    AColor res;
                    res.r = glm::mix(dst.r, finalColor.r, maskColor.r * finalColor.a);
                    res.g = glm::mix(dst.g, finalColor.g, maskColor.g * finalColor.a);
                    res.b = glm::mix(dst.b, finalColor.b, maskColor.b * finalColor.a);
                    res.a = dst.a;
                    
                    mContext->putPixel(glm::uvec2(x, y), glm::u8vec4(res * 255.f));
                }
            }
        }
    }
}
void SoftwareRenderer::lines(const ADisplayList::Lines& v, const glm::mat4& transform, Blending blending) {}
void SoftwareRenderer::points(const ADisplayList::Points& v, const glm::mat4& transform, Blending blending) {}
void SoftwareRenderer::lines(const ADisplayList::LineBatches& v, const glm::mat4& transform, Blending blending) {}
void SoftwareRenderer::squareSector(const ADisplayList::SquareSector& v, const glm::mat4& transform, Blending blending) {}
void SoftwareRenderer::backdrops(const ADisplayList::Backdrop& v, const glm::mat4& transform) {}
void SoftwareRenderer::backdrops(glm::ivec2 fbSize, glm::ivec2 size, std::span<const ass::Backdrop::Preprocessed> backdrops) {}

glm::mat4 SoftwareRenderer::getProjectionMatrix() const { return glm::mat4(1.0f); }

_<ITexture> SoftwareRenderer::createTexture(glm::u32vec2 size, APixelFormat format) {
    auto t = _new<SoftwareTexture>();
    t->upload(AImage(size, format));
    return t;
}

_unique<IRenderViewToTexture> SoftwareRenderer::newRenderViewToTexture() noexcept { return nullptr; }
