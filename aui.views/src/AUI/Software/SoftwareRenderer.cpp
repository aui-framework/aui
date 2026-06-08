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
#include <AUI/Hash.h>
#include <AUI/Traits/values.h>
#include <AUI/Render/CommonOffscreenRenderPass.h>
#include <bit>
#include <cmath>

_unique<IOffscreenRenderPass> SoftwareRenderer::beginOffscreen(const _<ITexture>& renderTarget) {
    return std::make_unique<CommonOffscreenRenderPass>(*this, renderTarget);
}

void SoftwareRenderer::endOffscreen(_unique<IOffscreenRenderPass> pass) {
    if (auto c = dynamic_cast<CommonOffscreenRenderPass*>(pass.get())) {
        c->displayList.optimize();
        c->displayList.draw(*this, c->target);
    }
}
#include <AUI/Image/AImage.h>
#include <AUI/Render/FontAtlas.hpp>
#include <AUI/Platform/AFontManager.h>
#include "SoftwareTexture.h"
#include <AUI/Render/Brush/Gradient.h>

namespace {
glm::vec4 toVec4(const ARect<float>& rect) {
    return { rect.p1.x, rect.p1.y, rect.size().x, rect.size().y };
}

float rectArea(const glm::vec4& rect) {
    return std::max(0.f, rect.z) * std::max(0.f, rect.w);
}

glm::vec4 intersectRects(const glm::vec4& a, const glm::vec4& b) {
    float x = std::max(a.x, b.x);
    float y = std::max(a.y, b.y);
    float w = std::min(a.x + a.z, b.x + b.z) - x;
    float h = std::min(a.y + a.w, b.y + b.w) - y;
    if (w <= 0.f || h <= 0.f) {
        return glm::vec4(0.f);
    }
    return { x, y, w, h };
}

void hashFloat(std::size_t& seed, float value) {
    aui::hash_combine(seed, std::bit_cast<uint32_t>(value));
}

void hashVec4(std::size_t& seed, const glm::vec4& value) {
    hashFloat(seed, value.x);
    hashFloat(seed, value.y);
    hashFloat(seed, value.z);
    hashFloat(seed, value.w);
}

std::size_t roundedRectMaskCacheKey(const ARect<float>& rect, float radius, bool inverted, const ARect<float>& bounds) {
    std::size_t seed = 0;
    hashVec4(seed, toVec4(rect));
    hashFloat(seed, radius);
    aui::hash_combine(seed, inverted);
    hashVec4(seed, toVec4(bounds));
    return seed;
}

std::size_t mergedMaskCacheKey(ITexture* mask1, const glm::vec4& mask1Rect, ITexture* mask2, const glm::vec4& mask2Rect) {
    std::size_t seed = 0;
    aui::hash_combine(seed, reinterpret_cast<std::uintptr_t>(mask1));
    hashVec4(seed, mask1Rect);
    aui::hash_combine(seed, reinterpret_cast<std::uintptr_t>(mask2));
    hashVec4(seed, mask2Rect);
    return seed;
}

glm::vec4 erf(glm::vec4 x) {
    glm::vec4 s = glm::sign(x);
    glm::vec4 a = glm::abs(x);
    x = glm::vec4(1.0f) + (glm::vec4(0.278393f) + (glm::vec4(0.230389f) + glm::vec4(0.078108f) * (a * a)) * a) * a;
    x = x * x;
    return s - s / (x * x);
}

float roundedRectSDF(glm::vec2 p, glm::vec2 size, float r) {
    glm::vec2 d = glm::abs(p - size * 0.5f) - (size * 0.5f - glm::vec2(r));
    return glm::length(glm::max(d, glm::vec2(0.f))) + glm::min(glm::max(d.x, d.y), 0.f) - r;
}

float roundedRectCoverage(glm::vec2 localPos, glm::vec2 size, float radius, float scale) {
    float sdf = roundedRectSDF(localPos, size, radius) * scale;
    return glm::clamp(0.5f - sdf, 0.f, 1.f);
}

float roundedRectBorderCoverage(glm::vec2 localPos, glm::vec2 size, float radius, float borderWidth, float scale) {
    float sdf_outer = roundedRectSDF(localPos, size, radius);
    float sdf_border = std::abs(sdf_outer + borderWidth * 0.5f) - borderWidth * 0.5f;
    return glm::clamp(0.5f - sdf_border * scale, 0.f, 1.f);
}

AColor sample(const AImage& img, glm::vec2 uv, TextureFilter filter) {
    if (img.width() == 0 || img.height() == 0) return AColor::BLACK;
    if (filter == TextureFilter::NEAREST) {
        int x = (int)(uv.x * (float)img.width());
        int y = (int)(uv.y * (float)img.height());
        return img.get({(uint32_t)glm::clamp(x, 0, (int)img.width() - 1), (uint32_t)glm::clamp(y, 0, (int)img.height() - 1)});
    }
    float tx = uv.x * (float)img.width() - 0.5f;
    float ty = uv.y * (float)img.height() - 0.5f;
    int x0 = (int)std::floor(tx);
    int y0 = (int)std::floor(ty);
    int x1 = x0 + 1;
    int y1 = y0 + 1;

    float fx = tx - (float)x0;
    float fy = ty - (float)y0;

    auto getSafe = [&](int x, int y) {
        return glm::vec4(img.get({(uint32_t)glm::clamp(x, 0, (int)img.width() - 1), (uint32_t)glm::clamp(y, 0, (int)img.height() - 1)}));
    };

    glm::vec4 c00 = getSafe(x0, y0);
    glm::vec4 c10 = getSafe(x1, y0);
    glm::vec4 c01 = getSafe(x0, y1);
    glm::vec4 c11 = getSafe(x1, y1);

    return glm::mix(glm::mix(c00, c10, fx), glm::mix(c01, c11, fx), fy);
}

AImage resizeLinear(AImageView source, glm::uvec2 newSize) {
    if (source.size() == newSize) {
        return AImage(source);
    }
    AImage resized(newSize, source.format());
    if (newSize.x == 0 || newSize.y == 0 || source.width() == 0 || source.height() == 0) {
        return resized;
    }

    auto sourceMax = glm::max(source.size(), glm::uvec2(1)) - glm::uvec2(1);
    auto ratio = glm::vec2(sourceMax) / glm::vec2(glm::max(newSize, glm::uvec2(1)));

    for (uint32_t y = 0; y < newSize.y; ++y) {
        for (uint32_t x = 0; x < newSize.x; ++x) {
            auto sourceX = ratio.x * float(x);
            auto sourceY = ratio.y * float(y);

            auto x0 = uint32_t(sourceX);
            auto y0 = uint32_t(sourceY);
            auto x1 = glm::min(x0 + 1, source.width() - 1);
            auto y1 = glm::min(y0 + 1, source.height() - 1);
            auto xWeight = sourceX - float(x0);
            auto yWeight = sourceY - float(y0);

            auto c00 = source.get({x0, y0}) * ((1.f - xWeight) * (1.f - yWeight));
            auto c10 = source.get({x1, y0}) * (xWeight * (1.f - yWeight));
            auto c01 = source.get({x0, y1}) * (yWeight * (1.f - xWeight));
            auto c11 = source.get({x1, y1}) * (xWeight * yWeight);
            resized.set({x, y}, c00 + c10 + c01 + c11);
        }
    }

    return resized;
}

void gaussianBlur(AImage& image, int radius) {
    if (radius <= 0 || image.width() == 0 || image.height() == 0) {
        return;
    }

    auto kernelSize = radius * 2 + 1;
    AVector<float> kernel;
    kernel.reserve(kernelSize);

    auto sigma = glm::max(radius * 0.5f, 1.f);
    auto denominator = 2.f * sigma * sigma;
    float kernelSum = 0.f;
    for (int i = -radius; i <= radius; ++i) {
        auto weight = std::exp(-(i * i) / denominator);
        kernel << weight;
        kernelSum += weight;
    }
    for (auto& weight : kernel) {
        weight /= kernelSum;
    }

    AImage horizontal(image.size(), image.format());
    for (uint32_t y = 0; y < image.height(); ++y) {
        for (uint32_t x = 0; x < image.width(); ++x) {
            glm::vec4 color(0.f);
            for (int i = -radius; i <= radius; ++i) {
                auto sampleX = glm::clamp(int(x) + i, 0, int(image.width()) - 1);
                color += glm::vec4(image.get({uint32_t(sampleX), y})) * kernel[i + radius];
            }
            horizontal.set({x, y}, AColor(glm::clamp(color, 0.f, 1.f)));
        }
    }

    AImage result(image.size(), image.format());
    for (uint32_t y = 0; y < image.height(); ++y) {
        for (uint32_t x = 0; x < image.width(); ++x) {
            glm::vec4 color(0.f);
            for (int i = -radius; i <= radius; ++i) {
                auto sampleY = glm::clamp(int(y) + i, 0, int(image.height()) - 1);
                color += glm::vec4(horizontal.get({x, uint32_t(sampleY)})) * kernel[i + radius];
            }
            result.set({x, y}, AColor(glm::clamp(color, 0.f, 1.f)));
        }
    }

    image = std::move(result);
}
}

SoftwareRenderer::SoftwareRenderer() :
    mFontCache(AFontManager::inst().createCache(this)) {}

void SoftwareRenderer::putPixel(glm::ivec2 pos, AColor color, const APaint& paint) {
    if (mRenderTarget && mRenderTarget->format() == APixelFormat::R8_UNORM) {
        color = AColor(color.a, 0.f, 0.f, color.a);
    }
    glm::uvec2 bitmapSize;
    if (mRenderTarget) {
        bitmapSize = mRenderTarget->size();
    } else if (mContext) {
        bitmapSize = mContext->bitmapSize();
    } else {
        return;
    }

    if (pos.x < 0 || pos.y < 0 || (uint32_t)pos.x >= bitmapSize.x || (uint32_t)pos.y >= bitmapSize.y) return;
    
    if (pos.x < mClipRect.p1.x || pos.y < mClipRect.p1.y || pos.x >= mClipRect.p2.x || pos.y >= mClipRect.p2.y) return;

    if (paint.blending != Blending::CLEAR) {
        float maskVal = 1.f;
        if (mMask) {
            auto s = _cast<SoftwareTexture>(mMask);
            if (s) {
                glm::vec2 texSize = s->getImage().size();
                glm::vec2 uv = (glm::vec2(pos) + 0.5f - glm::vec2(mMaskRect.x, mMaskRect.y)) / texSize;
                maskVal = glm::vec4(sample(s->getImage(), uv, TextureFilter::LINEAR)).r;
            }
        }
        if (maskVal <= 0.001f) return;
        color = color * maskVal;
    }
    
    AColor colorWithMask = color;

    glm::vec4 dst;
    if (mRenderTarget) {
        dst = glm::vec4(mRenderTarget->get(glm::uvec2(pos)));
    } else {
        dst = glm::vec4(mContext->getPixel(glm::uvec2(pos))) / 255.f;
    }

    AColor combined;
    if (paint.blending == Blending::CLEAR) {
        combined = colorWithMask;
    } else if (paint.blending == Blending::INVERSE_DST) {
        combined.r = colorWithMask.r * (1.f - dst.r);
        combined.g = colorWithMask.g * (1.f - dst.g);
        combined.b = colorWithMask.b * (1.f - dst.b);
        combined.a = colorWithMask.a * (1.f - dst.a) + dst.a;
    } else if (paint.blending == Blending::ADDITIVE) {
        combined.r = colorWithMask.r + dst.r;
        combined.g = colorWithMask.g + dst.g;
        combined.b = colorWithMask.b + dst.b;
        combined.a = colorWithMask.a * (1.f - dst.a) + dst.a;
    } else if (paint.blending == Blending::INVERSE_SRC) {
        combined.r = dst.r * (1.f - colorWithMask.r);
        combined.g = dst.g * (1.f - colorWithMask.g);
        combined.b = dst.b * (1.f - colorWithMask.b);
        combined.a = dst.a * (1.f - colorWithMask.a);
    } else {
        combined = dst * (1.f - colorWithMask.a) + colorWithMask;
    }

    if (mRenderTarget) {
        mRenderTarget->set(glm::uvec2(pos), AColor(glm::clamp(combined, 0.f, 1.f)));
    } else {
        mContext->putPixel(glm::uvec2(pos), glm::u8vec4(glm::clamp(combined, 0.f, 1.f) * 255.f));
    }
}

void SoftwareRenderer::solidRectangles(const ADrawList::SolidRectangles& v, const glm::mat4& transform, const APaint& paint) {
    glm::mat4 invTransform = glm::inverse(transform);
    for (const auto& inst : v.instances) {
        auto p1 = transform * glm::vec4(inst.position, 0.f, 1.f);
        auto p2 = transform * glm::vec4(inst.position + glm::vec2(inst.size.x, 0.f), 0.f, 1.f);
        auto p3 = transform * glm::vec4(inst.position + glm::vec2(0.f, inst.size.y), 0.f, 1.f);
        auto p4 = transform * glm::vec4(inst.position + inst.size, 0.f, 1.f);

        float minX = std::floor(std::min({p1.x, p2.x, p3.x, p4.x}));
        float maxX = std::ceil(std::max({p1.x, p2.x, p3.x, p4.x}));
        float minY = std::floor(std::min({p1.y, p2.y, p3.y, p4.y}));
        float maxY = std::ceil(std::max({p1.y, p2.y, p3.y, p4.y}));

        AColor color = inst.color.premultiply();
        for (int y = (int)minY; y < (int)maxY; ++y) {
            for (int x = (int)minX; x < (int)maxX; ++x) {
                glm::vec4 localPos4 = invTransform * glm::vec4((float)x + 0.5f, (float)y + 0.5f, 0.f, 1.f);
                glm::vec2 localPos = glm::vec2(localPos4.x, localPos4.y);
                if (localPos.x >= inst.position.x && localPos.x <= inst.position.x + inst.size.x &&
                    localPos.y >= inst.position.y && localPos.y <= inst.position.y + inst.size.y) {
                    putPixel({x, y}, color, paint);
                }
            }
        }
    }
}
void SoftwareRenderer::drawLine(glm::ivec2 p0, glm::ivec2 p1, float width, AColor color, const APaint& paint) {
    int dx = std::abs(p1.x - p0.x);
    int dy = std::abs(p1.y - p0.y);
    int sx = (p0.x < p1.x) ? 1 : -1;
    int sy = (p0.y < p1.y) ? 1 : -1;
    int err = dx - dy;
    int w = (int)std::round(width);
    if (w < 1) w = 1;

    while (true) {
        if (w == 1) {
            putPixel(p0, color, paint);
        } else {
            int r = w / 2;
            for (int dy_brush = -r; dy_brush < w - r; ++dy_brush) {
                for (int dx_brush = -r; dx_brush < w - r; ++dx_brush) {
                    putPixel({p0.x + dx_brush, p0.y + dy_brush}, color, paint);
                }
            }
        }
        if (p0.x == p1.x && p0.y == p1.y) break;
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            p0.x += sx;
        }
        if (e2 < dx) {
            err += dx;
            p0.y += sy;
        }
    }
}
void SoftwareRenderer::gradientRectangles(const ADrawList::GradientRectangles& v, const glm::mat4& transform, const APaint& paint) {
    if (v.instances.empty()) return;
    ALinearGradientBrush brush {
        v.colors,
        v.rotation
    };
    aui::render::brush::gradient::Helper helper(brush);
    glm::vec4 c1 = v.colors.size() > 0 ? v.colors[0].color.premultiply() : glm::vec4(0.f);
    glm::vec4 c2 = v.colors.size() > 1 ? v.colors[1].color.premultiply() : (v.colors.size() > 0 ? v.colors[0].color.premultiply() : glm::vec4(0.f));

    glm::mat4 invTransform = glm::inverse(transform);
    for (const auto& inst : v.instances) {
        auto p1 = transform * glm::vec4(inst.position, 0.f, 1.f);
        auto p2 = transform * glm::vec4(inst.position + glm::vec2(inst.size.x, 0.f), 0.f, 1.f);
        auto p3 = transform * glm::vec4(inst.position + glm::vec2(0.f, inst.size.y), 0.f, 1.f);
        auto p4 = transform * glm::vec4(inst.position + inst.size, 0.f, 1.f);

        float minX = std::floor(std::min({p1.x, p2.x, p3.x, p4.x}));
        float maxX = std::ceil(std::max({p1.x, p2.x, p3.x, p4.x}));
        float minY = std::floor(std::min({p1.y, p2.y, p3.y, p4.y}));
        float maxY = std::ceil(std::max({p1.y, p2.y, p3.y, p4.y}));

        for (int y = (int)minY; y < (int)maxY; ++y) {
            for (int x = (int)minX; x < (int)maxX; ++x) {
                glm::vec4 localPos4 = invTransform * glm::vec4((float)x + 0.5f, (float)y + 0.5f, 0.f, 1.f);
                glm::vec2 localPos = glm::vec2(localPos4.x, localPos4.y);
                if (localPos.x >= inst.position.x && localPos.x <= inst.position.x + inst.size.x &&
                    localPos.y >= inst.position.y && localPos.y <= inst.position.y + inst.size.y) {
                    glm::vec2 uv = (localPos - inst.position) / inst.size;
                    glm::vec3 transformedUv = helper.matrix * glm::vec3(uv, 1.f);
                    float t = glm::clamp(transformedUv.x, 0.f, 1.f);
                    AColor color = glm::mix(c1, c2, t) * inst.color.premultiply();
                    putPixel({x, y}, color, paint);
                }
            }
        }
    }
}
void SoftwareRenderer::texturedRectangles(const ADrawList::TexturedRectangles& v, const glm::mat4& transform, const APaint& paint) {
    auto texture = dynamic_cast<SoftwareTexture*>(v.texture.get());
    if (!texture) return;
    const auto& img = texture->getImage();
    if (img.width() == 0 || img.height() == 0) return;

    glm::mat4 invTransform = glm::inverse(transform);
    for (const auto& inst : v.instances) {
        auto p1 = transform * glm::vec4(inst.position, 0.f, 1.f);
        auto p2 = transform * glm::vec4(inst.position + glm::vec2(inst.size.x, 0.f), 0.f, 1.f);
        auto p3 = transform * glm::vec4(inst.position + glm::vec2(0.f, inst.size.y), 0.f, 1.f);
        auto p4 = transform * glm::vec4(inst.position + inst.size, 0.f, 1.f);

        float minX = std::floor(std::min({p1.x, p2.x, p3.x, p4.x}));
        float maxX = std::ceil(std::max({p1.x, p2.x, p3.x, p4.x}));
        float minY = std::floor(std::min({p1.y, p2.y, p3.y, p4.y}));
        float maxY = std::ceil(std::max({p1.y, p2.y, p3.y, p4.y}));

        for (int y = (int)minY; y < (int)maxY; ++y) {
            for (int x = (int)minX; x < (int)maxX; ++x) {
                glm::vec4 localPos4 = invTransform * glm::vec4((float)x + 0.5f, (float)y + 0.5f, 0.f, 1.f);
                glm::vec2 localPos = glm::vec2(localPos4.x, localPos4.y);
                if (localPos.x >= inst.position.x && localPos.x <= inst.position.x + inst.size.x &&
                    localPos.y >= inst.position.y && localPos.y <= inst.position.y + inst.size.y) {
                    glm::vec2 uv = (localPos - inst.position) / inst.size;
                    glm::vec2 texUv = glm::mix(v.uv1, v.uv2, uv);

                    AColor texColor = sample(img, texUv, texture->getFilter());
                    if (!v.premultiplied) {
                        texColor = texColor.premultiply();
                    }
                    putPixel({x, y}, texColor * inst.color, paint);
                }
            }
        }
    }
}
void SoftwareRenderer::solidRoundedRectangles(const ADrawList::SolidRoundedRectangles& v, const glm::mat4& transform, const APaint& paint) {
    glm::mat4 invTransform = glm::inverse(transform);
    float scale = glm::length(glm::vec2(transform * glm::vec4(1.f, 0.f, 0.f, 0.f)));
    for (const auto& inst : v.instances) {
        auto p1 = transform * glm::vec4(inst.position, 0.f, 1.f);
        auto p2 = transform * glm::vec4(inst.position + glm::vec2(inst.size.x, 0.f), 0.f, 1.f);
        auto p3 = transform * glm::vec4(inst.position + glm::vec2(0.f, inst.size.y), 0.f, 1.f);
        auto p4 = transform * glm::vec4(inst.position + inst.size, 0.f, 1.f);

        float minX = std::floor(std::min({p1.x, p2.x, p3.x, p4.x}));
        float maxX = std::ceil(std::max({p1.x, p2.x, p3.x, p4.x}));
        float minY = std::floor(std::min({p1.y, p2.y, p3.y, p4.y}));
        float maxY = std::ceil(std::max({p1.y, p2.y, p3.y, p4.y}));

        AColor color = inst.color.premultiply();
        float radius = v.radius;
        float maxRadius = std::min(inst.size.x, inst.size.y) * 0.5f;
        if (radius > maxRadius) {
            radius = maxRadius;
        }

        for (int y = (int)minY; y < (int)maxY; ++y) {
            for (int x = (int)minX; x < (int)maxX; ++x) {
                glm::vec4 localPos4 = invTransform * glm::vec4((float)x + 0.5f, (float)y + 0.5f, 0.f, 1.f);
                glm::vec2 localPos = glm::vec2(localPos4.x, localPos4.y) - inst.position;
                float a = roundedRectCoverage(localPos, inst.size, radius, scale);
                if (a > 0.001f) {
                    putPixel({x, y}, color * a, paint);
                }
            }
        }
    }
}
void SoftwareRenderer::gradientRoundedRectangles(const ADrawList::GradientRoundedRectangles& v, const glm::mat4& transform, const APaint& paint) {
    if (v.instances.empty()) return;
    ALinearGradientBrush brush {
        v.colors,
        v.rotation
    };
    aui::render::brush::gradient::Helper helper(brush);
    glm::vec4 c1 = v.colors.size() > 0 ? v.colors[0].color.premultiply() : glm::vec4(0.f);
    glm::vec4 c2 = v.colors.size() > 1 ? v.colors[1].color.premultiply() : (v.colors.size() > 0 ? v.colors[0].color.premultiply() : glm::vec4(0.f));

    glm::mat4 invTransform = glm::inverse(transform);
    float scale = glm::length(glm::vec2(transform * glm::vec4(1.f, 0.f, 0.f, 0.f)));
    for (const auto& inst : v.instances) {
        auto p1 = transform * glm::vec4(inst.position, 0.f, 1.f);
        auto p2 = transform * glm::vec4(inst.position + glm::vec2(inst.size.x, 0.f), 0.f, 1.f);
        auto p3 = transform * glm::vec4(inst.position + glm::vec2(0.f, inst.size.y), 0.f, 1.f);
        auto p4 = transform * glm::vec4(inst.position + inst.size, 0.f, 1.f);

        float minX = std::floor(std::min({p1.x, p2.x, p3.x, p4.x}));
        float maxX = std::ceil(std::max({p1.x, p2.x, p3.x, p4.x}));
        float minY = std::floor(std::min({p1.y, p2.y, p3.y, p4.y}));
        float maxY = std::ceil(std::max({p1.y, p2.y, p3.y, p4.y}));

        AColor color = inst.color.premultiply();
        float radius = v.radius;
        float maxRadius = std::min(inst.size.x, inst.size.y) * 0.5f;
        if (radius > maxRadius) {
            radius = maxRadius;
        }

        for (int y = (int)minY; y < (int)maxY; ++y) {
            for (int x = (int)minX; x < (int)maxX; ++x) {
                glm::vec4 localPos4 = invTransform * glm::vec4((float)x + 0.5f, (float)y + 0.5f, 0.f, 1.f);
                glm::vec2 localPos = glm::vec2(localPos4.x, localPos4.y) - inst.position;
                float a = roundedRectCoverage(localPos, inst.size, radius, scale);
                if (a > 0.001f) {
                    glm::vec2 uv = localPos / inst.size;
                    glm::vec3 transformedUv = helper.matrix * glm::vec3(uv, 1.f);
                    float t = glm::clamp(transformedUv.x, 0.f, 1.f);
                    AColor gradColor = glm::mix(c1, c2, t);
                    putPixel({x, y}, color * gradColor * a, paint);
                }
            }
        }
    }
}
void SoftwareRenderer::texturedRoundedRectangles(const ADrawList::TexturedRoundedRectangles& v, const glm::mat4& transform, const APaint& paint) {
    auto texture = dynamic_cast<SoftwareTexture*>(v.texture.get());
    if (!texture) return;
    const auto& img = texture->getImage();
    if (img.width() == 0 || img.height() == 0) return;

    glm::mat4 invTransform = glm::inverse(transform);
    float scale = glm::length(glm::vec2(transform * glm::vec4(1.f, 0.f, 0.f, 0.f)));
    for (const auto& inst : v.instances) {
        auto p1 = transform * glm::vec4(inst.position, 0.f, 1.f);
        auto p2 = transform * glm::vec4(inst.position + glm::vec2(inst.size.x, 0.f), 0.f, 1.f);
        auto p3 = transform * glm::vec4(inst.position + glm::vec2(0.f, inst.size.y), 0.f, 1.f);
        auto p4 = transform * glm::vec4(inst.position + inst.size, 0.f, 1.f);

        float minX = std::floor(std::min({p1.x, p2.x, p3.x, p4.x}));
        float maxX = std::ceil(std::max({p1.x, p2.x, p3.x, p4.x}));
        float minY = std::floor(std::min({p1.y, p2.y, p3.y, p4.y}));
        float maxY = std::ceil(std::max({p1.y, p2.y, p3.y, p4.y}));

        AColor color = inst.color.premultiply();
        float radius = v.radius;
        float maxRadius = std::min(inst.size.x, inst.size.y) * 0.5f;
        if (radius > maxRadius) {
            radius = maxRadius;
        }

        for (int y = (int)minY; y < (int)maxY; ++y) {
            for (int x = (int)minX; x < (int)maxX; ++x) {
                glm::vec4 localPos4 = invTransform * glm::vec4((float)x + 0.5f, (float)y + 0.5f, 0.f, 1.f);
                glm::vec2 localPos = glm::vec2(localPos4.x, localPos4.y) - inst.position;
                float a = roundedRectCoverage(localPos, inst.size, radius, scale);
                if (a > 0.001f) {
                    glm::vec2 uv = localPos / inst.size;
                    glm::vec2 texUv = glm::mix(v.uv1, v.uv2, uv);

                    AColor texColor = sample(img, texUv, texture->getFilter());
                    if (!v.premultiplied) {
                        texColor = texColor.premultiply();
                    }
                    putPixel({x, y}, texColor * color * a, paint);
                }
            }
        }
    }
}
void SoftwareRenderer::rectangleBorders(const ADrawList::RectangleBorders& v, const glm::mat4& transform, const APaint& paint) {
    glm::mat4 invTransform = glm::inverse(transform);
    for (const auto& inst : v.instances) {
        auto p1 = transform * glm::vec4(inst.position, 0.f, 1.f);
        auto p2 = transform * glm::vec4(inst.position + glm::vec2(inst.size.x, 0.f), 0.f, 1.f);
        auto p3 = transform * glm::vec4(inst.position + glm::vec2(0.f, inst.size.y), 0.f, 1.f);
        auto p4 = transform * glm::vec4(inst.position + inst.size, 0.f, 1.f);

        float minX = std::floor(std::min({p1.x, p2.x, p3.x, p4.x}));
        float maxX = std::ceil(std::max({p1.x, p2.x, p3.x, p4.x}));
        float minY = std::floor(std::min({p1.y, p2.y, p3.y, p4.y}));
        float maxY = std::ceil(std::max({p1.y, p2.y, p3.y, p4.y}));

        AColor color = inst.color.premultiply();
        for (int y = (int)minY; y < (int)maxY; ++y) {
            for (int x = (int)minX; x < (int)maxX; ++x) {
                glm::vec4 localPos4 = invTransform * glm::vec4((float)x + 0.5f, (float)y + 0.5f, 0.f, 1.f);
                glm::vec2 localPos = glm::vec2(localPos4.x, localPos4.y);
                if (localPos.x >= inst.position.x && localPos.x <= inst.position.x + inst.size.x &&
                    localPos.y >= inst.position.y && localPos.y <= inst.position.y + inst.size.y) {

                    bool inner = localPos.x >= inst.position.x + v.lineWidth && localPos.x <= inst.position.x + inst.size.x - v.lineWidth &&
                                 localPos.y >= inst.position.y + v.lineWidth && localPos.y <= inst.position.y + inst.size.y - v.lineWidth;
                    if (!inner) {
                        putPixel({x, y}, color, paint);
                    }
                }
            }
        }
    }
}
void SoftwareRenderer::roundedRectangleBorders(const ADrawList::RoundedRectangleBorders& v, const glm::mat4& transform, const APaint& paint) {
    glm::mat4 invTransform = glm::inverse(transform);
    float scale = glm::length(glm::vec2(transform * glm::vec4(1.f, 0.f, 0.f, 0.f)));
    for (const auto& inst : v.instances) {
        auto p1 = transform * glm::vec4(inst.position, 0.f, 1.f);
        auto p2 = transform * glm::vec4(inst.position + glm::vec2(inst.size.x, 0.f), 0.f, 1.f);
        auto p3 = transform * glm::vec4(inst.position + glm::vec2(0.f, inst.size.y), 0.f, 1.f);
        auto p4 = transform * glm::vec4(inst.position + inst.size, 0.f, 1.f);

        float minX = std::floor(std::min({p1.x, p2.x, p3.x, p4.x}));
        float maxX = std::ceil(std::max({p1.x, p2.x, p3.x, p4.x}));
        float minY = std::floor(std::min({p1.y, p2.y, p3.y, p4.y}));
        float maxY = std::ceil(std::max({p1.y, p2.y, p3.y, p4.y}));

        AColor color = inst.color.premultiply();
        float radius = v.radius;
        float maxRadius = std::min(inst.size.x, inst.size.y) * 0.5f;
        if (radius > maxRadius) {
            radius = maxRadius;
        }

        for (int y = (int)minY; y < (int)maxY; ++y) {
            for (int x = (int)minX; x < (int)maxX; ++x) {
                glm::vec4 localPos4 = invTransform * glm::vec4((float)x + 0.5f, (float)y + 0.5f, 0.f, 1.f);
                glm::vec2 localPos = glm::vec2(localPos4.x, localPos4.y) - inst.position;
                float a = roundedRectBorderCoverage(localPos, inst.size, radius, (float)v.borderWidth, scale);
                if (a > 0.001f) {
                    putPixel({x, y}, color * a, paint);
                }
            }
        }
    }
}
void SoftwareRenderer::boxShadow(const ADrawList::BoxShadow& v, const glm::mat4& transform, const APaint& paint) {
    float sigma = v.blurRadius / 2.f;
    float padding = v.blurRadius * 2.f;
    glm::vec2 pos = v.position - padding;
    glm::vec2 size = v.size + padding * 2.f;

    auto p1 = transform * glm::vec4(pos, 0.f, 1.f);
    auto p2 = transform * glm::vec4(pos + glm::vec2(size.x, 0.f), 0.f, 1.f);
    auto p3 = transform * glm::vec4(pos + glm::vec2(0.f, size.y), 0.f, 1.f);
    auto p4 = transform * glm::vec4(pos + size, 0.f, 1.f);

    float minX = std::floor(std::min({p1.x, p2.x, p3.x, p4.x}));
    float maxX = std::ceil(std::max({p1.x, p2.x, p3.x, p4.x}));
    float minY = std::floor(std::min({p1.y, p2.y, p3.y, p4.y}));
    float maxY = std::ceil(std::max({p1.y, p2.y, p3.y, p4.y}));

    glm::mat4 invTransform = glm::inverse(transform);
    AColor color = v.color.premultiply();

    for (int y = (int)minY; y < (int)maxY; ++y) {
        for (int x = (int)minX; x < (int)maxX; ++x) {
            glm::vec4 localPos4 = invTransform * glm::vec4((float)x + 0.5f, (float)y + 0.5f, 0.f, 1.f);
            glm::vec2 val = glm::vec2(localPos4.x, localPos4.y);
            glm::vec4 query = glm::vec4(val - v.position, val - (v.position + v.size));
            glm::vec4 erfVal = erf(query * (std::sqrt(0.5f) / sigma));
            glm::vec4 integral = glm::vec4(0.5f) + glm::vec4(0.5f) * erfVal;
            float shadowFactor = glm::clamp((integral.z - integral.x) * (integral.w - integral.y), 0.f, 1.f);
            if (shadowFactor > 0.001f) {
                putPixel({x, y}, color * shadowFactor, paint);
            }
        }
    }
}
void SoftwareRenderer::boxShadowInner(const ADrawList::BoxShadowInner& v, const glm::mat4& transform, const APaint& paint) {
    float sigma = v.blurRadius / 2.f;
    glm::vec2 lower = v.position + v.offset + glm::vec2(v.spreadRadius);
    glm::vec2 upper = v.position + v.size + v.offset - glm::vec2(v.spreadRadius);

    auto p1 = transform * glm::vec4(v.position, 0.f, 1.f);
    auto p2 = transform * glm::vec4(v.position + glm::vec2(v.size.x, 0.f), 0.f, 1.f);
    auto p3 = transform * glm::vec4(v.position + glm::vec2(0.f, v.size.y), 0.f, 1.f);
    auto p4 = transform * glm::vec4(v.position + v.size, 0.f, 1.f);

    float minX = std::floor(std::min({p1.x, p2.x, p3.x, p4.x}));
    float maxX = std::ceil(std::max({p1.x, p2.x, p3.x, p4.x}));
    float minY = std::floor(std::min({p1.y, p2.y, p3.y, p4.y}));
    float maxY = std::ceil(std::max({p1.y, p2.y, p3.y, p4.y}));

    glm::mat4 invTransform = glm::inverse(transform);
    float scale = glm::length(glm::vec2(transform * glm::vec4(1.f, 0.f, 0.f, 0.f)));
    AColor color = v.color.premultiply();

    for (int y = (int)minY; y < (int)maxY; ++y) {
        for (int x = (int)minX; x < (int)maxX; ++x) {
            glm::vec4 localPos4 = invTransform * glm::vec4((float)x + 0.5f, (float)y + 0.5f, 0.f, 1.f);
            glm::vec2 val = glm::vec2(localPos4.x, localPos4.y);

            glm::vec2 localPos = val - v.position;
            float a = roundedRectCoverage(localPos, v.size, (float)v.borderRadius, scale);

            glm::vec4 query = glm::vec4(val - lower, val - upper);
            glm::vec4 erfVal = erf(query * (std::sqrt(0.5f) / sigma));
            glm::vec4 integral = glm::vec4(0.5f) + glm::vec4(0.5f) * erfVal;
            float shadowFactor = glm::clamp((integral.z - integral.x) * (integral.w - integral.y), 0.f, 1.f);
            float factor = (1.f - shadowFactor) * a;

            if (factor > 0.001f) {
                putPixel({x, y}, color * factor, paint);
            }
        }
    }
}
void SoftwareRenderer::glyphs(const ADrawList::Glyphs& v, const glm::mat4& transform, const APaint& paint) {
    auto texture = dynamic_cast<SoftwareTexture*>(v.texture.get());
    if (!texture)
        return;
    const auto& img = texture->getImage();
    if (img.width() == 0)
        return;

    glm::mat4 invTransform = glm::inverse(transform);
    for (const auto& inst : v.instances) {
        auto p1 = transform * glm::vec4(inst.position, 0.f, 1.f);
        auto p2 = transform * glm::vec4(inst.position + glm::vec2(inst.size.x, 0.f), 0.f, 1.f);
        auto p3 = transform * glm::vec4(inst.position + glm::vec2(0.f, inst.size.y), 0.f, 1.f);
        auto p4 = transform * glm::vec4(inst.position + inst.size, 0.f, 1.f);

        float minX = std::floor(std::min({p1.x, p2.x, p3.x, p4.x}));
        float maxX = std::ceil(std::max({p1.x, p2.x, p3.x, p4.x}));
        float minY = std::floor(std::min({p1.y, p2.y, p3.y, p4.y}));
        float maxY = std::ceil(std::max({p1.y, p2.y, p3.y, p4.y}));

        for (int y = (int)minY; y < (int)maxY; ++y) {
            for (int x = (int)minX; x < (int)maxX; ++x) {
                glm::vec4 localPos4 = invTransform * glm::vec4((float)x + 0.5f, (float)y + 0.5f, 0.f, 1.f);
                glm::vec2 localPos = glm::vec2(localPos4.x, localPos4.y);
                if (localPos.x >= inst.position.x && localPos.x <= inst.position.x + inst.size.x &&
                    localPos.y >= inst.position.y && localPos.y <= inst.position.y + inst.size.y) {

                    glm::vec2 uv = (localPos - inst.position) / inst.size;
                    glm::vec2 texUv = glm::mix(inst.u1, inst.u2, uv);

                    int texX = glm::clamp((int)(texUv.x * (float)img.width()), 0, (int)img.width() - 1);
                    int texY = glm::clamp((int)(texUv.y * (float)img.height()), 0, (int)img.height() - 1);

                    AColor maskColor = img.get({(uint32_t)texX, (uint32_t)texY});
                    AColor pColor = inst.color.premultiply();

                    if (!v.isSubpixel) {
                        AColor finalColor = pColor * maskColor.r;
                        putPixel({x, y}, finalColor, paint);
                    } else {
                        glm::uvec2 bitmapSize;
                        if (mRenderTarget) {
                            bitmapSize = mRenderTarget->size();
                        } else if (mContext) {
                            bitmapSize = mContext->bitmapSize();
                        } else {
                            continue;
                        }

                        if (x < 0 || y < 0 || (uint32_t)x >= bitmapSize.x || (uint32_t)y >= bitmapSize.y) continue;

                        float globalMaskVal = 1.f;
                        if (mMask) {
                            auto s = _cast<SoftwareTexture>(mMask);
                            if (s) {
                                glm::vec2 uv = (glm::vec2(x, y) + 0.5f - glm::vec2(mMaskRect.x, mMaskRect.y)) / glm::vec2(s->getImage().size());
                                globalMaskVal = glm::vec4(sample(s->getImage(), uv, TextureFilter::LINEAR)).r;
                            }
                        }

                        glm::vec4 dst;
                        if (mRenderTarget) {
                            dst = glm::vec4(mRenderTarget->get(glm::uvec2(x, y)));
                        } else {
                            dst = glm::vec4(mContext->getPixel(glm::uvec2(x, y))) / 255.f;
                        }

                        AColor res;
                        glm::vec3 mask = glm::vec3(maskColor) * globalMaskVal;
                        res.r = dst.r * (1.f - mask.r * pColor.a) + pColor.r * mask.r;
                        res.g = dst.g * (1.f - mask.g * pColor.a) + pColor.g * mask.g;
                        res.b = dst.b * (1.f - mask.b * pColor.a) + pColor.b * mask.b;
                        float avgMask = (mask.r + mask.g + mask.b) / 3.f;
                        res.a = dst.a * (1.f - avgMask * pColor.a) + avgMask * pColor.a;
                        if (mRenderTarget) {
                            mRenderTarget->set(glm::uvec2(x, y), AColor(glm::clamp(res, 0.f, 1.f)));
                        } else {
                            mContext->putPixel(glm::uvec2(x, y), glm::u8vec4(glm::clamp(res, 0.f, 1.f) * 255.f));
                        }
                    }
                }
            }
        }
    }
}

_<IRenderer::IMultiStringCanvas> SoftwareRenderer::newMultiStringCanvas(const AFontStyle& style) {
    auto entryData = aui::getFontEntryData(style, mFontCache);
    return _new<aui::MultiStringCanvas>(*this, entryData, style);
}
_<IRenderer::IPrerenderedString> SoftwareRenderer::prerenderString(glm::vec2 position, const AString& text, const AFontStyle& fs) {
    if (text.empty()) return nullptr;
    auto c = newMultiStringCanvas(fs);
    c->addString(position, text);
    return c->finalize();
}
void SoftwareRenderer::lines(const ADrawList::Lines& v, const glm::mat4& transform, const APaint& paint) {
    if (v.points.size() < 2) return;
    float widthPx = v.width.getValuePx();
    AColor color = paint.color.premultiply();

    for (size_t i = 1; i < v.points.size(); ++i) {
        auto p0 = transform * glm::vec4(v.points[i-1], 0.f, 1.f);
        auto p1 = transform * glm::vec4(v.points[i], 0.f, 1.f);
        drawLine({(int)p0.x, (int)p0.y}, {(int)p1.x, (int)p1.y}, widthPx, color, paint);
    }
}
void SoftwareRenderer::points(const ADrawList::Points& v, const glm::mat4& transform, const APaint& paint) {
    if (v.points.empty()) return;
    float size = v.size.getValuePx();
    int w = (int)std::round(size);
    if (w < 1) w = 1;
    int r = w / 2;
    AColor color = paint.color.premultiply();

    for (const auto& pt : v.points) {
        auto p = transform * glm::vec4(pt, 0.f, 1.f);
        glm::ivec2 center = { (int)p.x, (int)p.y };
        for (int dy = -r; dy < w - r; ++dy) {
            for (int dx = -r; dx < w - r; ++dx) {
                putPixel({center.x + dx, center.y + dy}, color, paint);
            }
        }
    }
}
void SoftwareRenderer::lines(const ADrawList::LineBatches& v, const glm::mat4& transform, const APaint& paint) {
    if (v.points.empty()) return;
    float widthPx = v.width.getValuePx();
    AColor color = paint.color.premultiply();

    for (const auto& [pt1, pt2] : v.points) {
        auto p0 = transform * glm::vec4(pt1, 0.f, 1.f);
        auto p1 = transform * glm::vec4(pt2, 0.f, 1.f);
        drawLine({(int)p0.x, (int)p0.y}, {(int)p1.x, (int)p1.y}, widthPx, color, paint);
    }
}
void SoftwareRenderer::squareSector(const ADrawList::SquareSector& v, const glm::mat4& transform, const APaint& paint) {
    auto p1 = transform * glm::vec4(v.position, 0.f, 1.f);
    auto p2 = transform * glm::vec4(v.position + glm::vec2(v.size.x, 0.f), 0.f, 1.f);
    auto p3 = transform * glm::vec4(v.position + glm::vec2(0.f, v.size.y), 0.f, 1.f);
    auto p4 = transform * glm::vec4(v.position + v.size, 0.f, 1.f);

    float minX = std::floor(std::min({p1.x, p2.x, p3.x, p4.x}));
    float maxX = std::ceil(std::max({p1.x, p2.x, p3.x, p4.x}));
    float minY = std::floor(std::min({p1.y, p2.y, p3.y, p4.y}));
    float maxY = std::ceil(std::max({p1.y, p2.y, p3.y, p4.y}));

    glm::mat4 invTransform = glm::inverse(transform);
    AColor color = paint.color.premultiply();

    float beginRad = v.begin.radians();
    float endRad = v.end.radians();
    const float PI2 = 6.28318530718f;

    if (endRad - beginRad >= PI2 - 0.0001f) {
        for (int y = (int)minY; y < (int)maxY; ++y) {
            for (int x = (int)minX; x < (int)maxX; ++x) {
                glm::vec4 localPos4 = invTransform * glm::vec4((float)x + 0.5f, (float)y + 0.5f, 0.f, 1.f);
                glm::vec2 localPos = glm::vec2(localPos4.x, localPos4.y);
                if (localPos.x >= v.position.x && localPos.x <= v.position.x + v.size.x &&
                    localPos.y >= v.position.y && localPos.y <= v.position.y + v.size.y) {
                    putPixel({x, y}, color, paint);
                }
            }
        }
        return;
    }

    auto normalize = [PI2](float a) {
        a = std::fmod(a, PI2);
        if (a < 0.f) a += PI2;
        return a;
    };

    float b = normalize(beginRad);
    float e = normalize(endRad);

    for (int y = (int)minY; y < (int)maxY; ++y) {
        for (int x = (int)minX; x < (int)maxX; ++x) {
            glm::vec4 localPos4 = invTransform * glm::vec4((float)x + 0.5f, (float)y + 0.5f, 0.f, 1.f);
            glm::vec2 localPos = glm::vec2(localPos4.x, localPos4.y);
            if (localPos.x >= v.position.x && localPos.x <= v.position.x + v.size.x &&
                localPos.y >= v.position.y && localPos.y <= v.position.y + v.size.y) {

                glm::vec2 uv = (localPos - v.position) / v.size;
                glm::vec2 centeredUv = uv * 2.f - 1.f;
                float angle = std::atan2(centeredUv.x, -centeredUv.y);
                if (angle < 0.f) angle += PI2;

                bool inside = false;
                if (b < e) {
                    inside = (angle >= b && angle <= e);
                } else {
                    inside = (angle >= b || angle <= e);
                }

                if (inside) {
                    putPixel({x, y}, color, paint);
                }
            }
        }
    }
}
void SoftwareRenderer::backdrops(glm::ivec2 position, glm::ivec2 size, std::span<const ass::Backdrop::Preprocessed> backdrops) {
    if (!mRenderTarget || backdrops.empty() || !glm::all(glm::greaterThan(size, glm::ivec2(0)))) {
        return;
    }

    auto clippedPosition = glm::max(position, glm::ivec2(0));
    auto clippedEnd = glm::min(position + size, glm::ivec2(mRenderTarget->size()));
    auto clippedSize = clippedEnd - clippedPosition;
    if (!glm::all(glm::greaterThan(clippedSize, glm::ivec2(0)))) {
        return;
    }

    AImage area = AImageView(*mRenderTarget).cropped(glm::uvec2(clippedPosition), glm::uvec2(clippedSize)).convert(APixelFormat::R8G8B8A8_UNORM);
    auto originalSize = area.size();

    for (const auto& backdrop : backdrops) {
        std::visit(aui::lambda_overloaded {
            [&](const ass::Backdrop::GaussianBlurCustom& blur) {
                auto downscale = glm::max(1, blur.downscale);
                auto scaledSize = glm::max(glm::uvec2(1), (originalSize + glm::uvec2(downscale - 1)) / uint32_t(downscale));
                auto working = downscale == 1 ? AImage(area) : resizeLinear(area, scaledSize);
                gaussianBlur(working, glm::max(1, int(std::lround(blur.radius.getValuePx()))));
                area = working.size() == originalSize ? std::move(working) : resizeLinear(working, originalSize);
            },
            [&](const auto&) {}
        }, backdrop);
    }

    for (uint32_t y = 0; y < area.height(); ++y) {
        for (uint32_t x = 0; x < area.width(); ++x) {
            putPixel(clippedPosition + glm::ivec2(x, y), area.get({x, y}), APaint {});
        }
    }
}

namespace {
class SoftwareFramebufferTexture : public ITexture {
public:
    SoftwareFramebufferTexture(glm::uvec2 size) : mSize(size) {}
    glm::u32vec2 getSize() const override { return mSize; }
    APixelFormat getFormat() const override { return APixelFormat::R8G8B8A8_UNORM; }
    void upload(AImageView image) override {}
private:
    glm::uvec2 mSize;
};
}

void SoftwareRenderer::setRenderTarget(const _<ITexture>& texture, glm::uvec2 size) {
    if (auto swTexture = dynamic_cast<SoftwareTexture*>(texture.get())) {
        mRenderTarget = const_cast<AImage*>(&swTexture->getImage());
    } else {
        mRenderTarget = nullptr;
    }
    mClipRect = { .p1 = {-1e10, -1e10}, .p2 = {1e10, 1e10} };
}

_<ITexture> SoftwareRenderer::createFramebufferWrapper(glm::uvec2 size) {
    return _new<SoftwareFramebufferTexture>(size);
}

void SoftwareRenderer::clear(const AColor& color) {
    if (mRenderTarget) {
        mRenderTarget->fill(color);
    }
}

glm::mat4 SoftwareRenderer::getProjectionMatrix() const { return glm::mat4(1.0f); }

_<ITexture> SoftwareRenderer::createTexture(glm::u32vec2 size, APixelFormat format, TextureFilter filter) {
    auto t = _new<SoftwareTexture>();
    t->upload(AImage(size, format));
    t->setFilter(filter);
    return t;
}

void SoftwareRenderer::setMask(const _<ITexture>& mask, const glm::vec4& maskRect) {
    mMask = mask;
    mMaskRect = maskRect;
}

_<ITexture> SoftwareRenderer::createRectMask(const ARect<float>& rect, bool inverted, const ARect<float>& bounds) {
    glm::u32vec2 size(std::max(1u, (unsigned)std::ceil(bounds.size().x)), std::max(1u, (unsigned)std::ceil(bounds.size().y)));
    auto destTexture = createTexture(size, APixelFormat::R8_UNORM);
    AImage destImg(size, APixelFormat::R8_UNORM);
    
    for (unsigned dy = 0; dy < size.y; ++dy) {
        for (unsigned dx = 0; dx < size.x; ++dx) {
            float ax = bounds.p1.x + dx + 0.5f;
            float ay = bounds.p1.y + dy + 0.5f;
            
            bool inside = ax >= rect.p1.x && ax <= rect.p2.x &&
                          ay >= rect.p1.y && ay <= rect.p2.y;
            
            float val = (inside ^ inverted) ? 1.f : 0.f;
            destImg.set({dx, dy}, AColor(val, 0.f, 0.f, 1.f));
        }
    }
    _cast<SoftwareTexture>(destTexture)->upload(std::move(destImg));
    return destTexture;
}

_<ITexture> SoftwareRenderer::createRoundedRectMask(const ARect<float>& rect, float radius, bool inverted, const ARect<float>& bounds) {
    const auto cacheKey = roundedRectMaskCacheKey(rect, radius, inverted, bounds);
    if (auto it = mRoundedRectMaskCache.find(cacheKey); it != mRoundedRectMaskCache.end()) {
        if (auto texture = it->second.texture.lock()) {
            return texture;
        }
        mRoundedRectMaskCache.erase(it);
    }

    auto usefulBounds = inverted ? bounds : rect.intersect(bounds);
    glm::u32vec2 size(std::max(1u, (unsigned)std::ceil(usefulBounds.size().x)), std::max(1u, (unsigned)std::ceil(usefulBounds.size().y)));
    auto destTexture = createTexture(size, APixelFormat::R8_UNORM);
    AImage destImg(size, APixelFormat::R8_UNORM);

    const auto rectMin = rect.min();
    const auto rectMax = rect.max();
    const auto rectSize = rect.size();
    radius = std::max(0.f, std::min(radius, std::min(rectSize.x, rectSize.y) * 0.5f));

    auto insideRoundedRect = [&](float ax, float ay) {
        auto qx = std::abs(ax - (rectMin.x + rectMax.x) * 0.5f) - (rectSize.x * 0.5f - radius);
        auto qy = std::abs(ay - (rectMin.y + rectMax.y) * 0.5f) - (rectSize.y * 0.5f - radius);
        auto outer = glm::length(glm::max(glm::vec2(qx, qy), glm::vec2(0.f)));
        auto inner = std::min(std::max(qx, qy), 0.f);
        return outer + inner <= radius;
    };

    for (unsigned dy = 0; dy < size.y; ++dy) {
        for (unsigned dx = 0; dx < size.x; ++dx) {
            float ax = usefulBounds.p1.x + dx + 0.5f;
            float ay = usefulBounds.p1.y + dy + 0.5f;
            float val = (insideRoundedRect(ax, ay) ^ inverted) ? 1.f : 0.f;
            destImg.set({dx, dy}, AColor(val, 0.f, 0.f, 1.f));
        }
    }

    _cast<SoftwareTexture>(destTexture)->upload(std::move(destImg));
    mRoundedRectMaskCache.emplace(cacheKey, RoundedRectMaskCacheEntry { .texture = destTexture });
    return destTexture;
}

IRendererBackend::AMergedMask SoftwareRenderer::mergeMasks(const _<ITexture>& mask1, const glm::vec4& mask1Rect,
                                                           const _<ITexture>& mask2, const glm::vec4& mask2Rect) {
    glm::vec4 mergedRect = intersectRects(mask1Rect, mask2Rect);

    if (rectArea(mergedRect) <= 0.f || !mask1 || !mask2) {
        auto emptyTexture = createTexture({1, 1}, APixelFormat::R8_UNORM);
        AImage img({1, 1}, APixelFormat::R8_UNORM);
        std::memset(img.modifiableBuffer().data(), 0, img.modifiableBuffer().getSize());
        _cast<SoftwareTexture>(emptyTexture)->upload(std::move(img));
        return { emptyTexture, glm::vec4(0.f) };
    }

    const auto cacheKey = mergedMaskCacheKey(mask1.get(), mask1Rect, mask2.get(), mask2Rect);
    if (auto it = mMergedMaskCache.find(cacheKey); it != mMergedMaskCache.end()) {
        if (auto texture = it->second.texture.lock()) {
            return { texture, it->second.rect };
        }
        mMergedMaskCache.erase(it);
    }

    glm::u32vec2 size(std::max(1u, (unsigned)std::ceil(mergedRect.z)), std::max(1u, (unsigned)std::ceil(mergedRect.w)));
    auto destTexture = createTexture(size, APixelFormat::R8_UNORM);
    AImage destImg(size, APixelFormat::R8_UNORM);
    std::memset(destImg.modifiableBuffer().data(), 0, destImg.modifiableBuffer().getSize());

    auto s1 = _cast<SoftwareTexture>(mask1);
    auto s2 = _cast<SoftwareTexture>(mask2);

    for (unsigned dy = 0; dy < size.y; ++dy) {
        for (unsigned dx = 0; dx < size.x; ++dx) {
            float ax = mergedRect.x + dx + 0.5f;
            float ay = mergedRect.y + dy + 0.5f;

            float val1 = 0.f;
            if (ax >= mask1Rect.x && ax <= mask1Rect.x + mask1Rect.z &&
                ay >= mask1Rect.y && ay <= mask1Rect.y + mask1Rect.w) {
                if (s1) {
                    glm::uvec2 m1Size = s1->getImage().size();
                    float tx = (ax - mask1Rect.x) / mask1Rect.z * m1Size.x;
                    float ty = (ay - mask1Rect.y) / mask1Rect.w * m1Size.y;
                    unsigned px = glm::clamp((unsigned)tx, 0u, m1Size.x - 1);
                    unsigned py = glm::clamp((unsigned)ty, 0u, m1Size.y - 1);
                    val1 = glm::vec4(s1->getImage().get({px, py})).r / 255.f;
                }
            }

            float val2 = 0.f;
            if (ax >= mask2Rect.x && ax <= mask2Rect.x + mask2Rect.z &&
                ay >= mask2Rect.y && ay <= mask2Rect.y + mask2Rect.w) {
                if (s2) {
                    glm::uvec2 m2Size = s2->getImage().size();
                    float tx = (ax - mask2Rect.x) / mask2Rect.z * m2Size.x;
                    float ty = (ay - mask2Rect.y) / mask2Rect.w * m2Size.y;
                    unsigned px = glm::clamp((unsigned)tx, 0u, m2Size.x - 1);
                    unsigned py = glm::clamp((unsigned)ty, 0u, m2Size.y - 1);
                    val2 = glm::vec4(s2->getImage().get({px, py})).r / 255.f;
                }
            }

            float finalVal = val1 * val2;
            destImg.set({dx, dy}, AColor(finalVal, 0.f, 0.f, 1.f));
        }
    }

    _cast<SoftwareTexture>(destTexture)->upload(std::move(destImg));
    AMergedMask merged { destTexture, glm::vec4(mergedRect.x, mergedRect.y, (float)size.x, (float)size.y) };
    mMergedMaskCache.emplace(cacheKey, MergedMaskCacheEntry { .texture = destTexture, .rect = merged.rect });
    return merged;
}

AImage SoftwareRenderer::readback(const _<ITexture>& texture) {
    if (auto softwareTexture = _cast<SoftwareTexture>(texture)) {
        return softwareTexture->getImage();
    }
    return {};
}
