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
#include <AUI/Render/FontAtlas.hpp>
#include <AUI/Platform/AFontManager.h>
#include "SoftwareTexture.h"
#include <AUI/Render/Brush/Gradient.h>

namespace {
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
}

SoftwareRenderer::SoftwareRenderer() :
    mFontCache(AFontManager::inst().createCache(this)) {}

void SoftwareRenderer::setWindow(ASurface* window) {
    mWindow = window;
    mContext = dynamic_cast<SoftwareRenderingContext*>(window->getRenderingContext().get());
}

void SoftwareRenderer::putPixel(glm::ivec2 pos, AColor color, const APaint& paint) {
    glm::uvec2 bitmapSize;
    if (mRenderTarget) {
        bitmapSize = mRenderTarget->size();
    } else if (mContext) {
        bitmapSize = mContext->bitmapSize();
    } else {
        return;
    }

    if (pos.x < 0 || pos.y < 0 || (uint32_t)pos.x >= bitmapSize.x || (uint32_t)pos.y >= bitmapSize.y) return;
    
    glm::vec4 dst;
    if (mRenderTarget) {
        dst = glm::vec4(mRenderTarget->get(glm::uvec2(pos))) / 255.f;
    } else {
        dst = glm::vec4(mContext->getPixel(glm::uvec2(pos))) / 255.f;
    }

    AColor combined;
    if (paint.blending == Blending::INVERSE_DST) {
        combined.r = color.r * (1.f - dst.r);
        combined.g = color.g * (1.f - dst.g);
        combined.b = color.b * (1.f - dst.b);
        combined.a = color.a * (1.f - dst.a) + dst.a;
    } else if (paint.blending == Blending::ADDITIVE) {
        combined.r = color.r + dst.r;
        combined.g = color.g + dst.g;
        combined.b = color.b + dst.b;
        combined.a = color.a * (1.f - dst.a) + dst.a;
    } else if (paint.blending == Blending::INVERSE_SRC) {
        combined.r = dst.r * (1.f - color.r);
        combined.g = dst.g * (1.f - color.g);
        combined.b = dst.b * (1.f - color.b);
        combined.a = color.a * (1.f - dst.a) + dst.a;
    } else {
        combined = dst * (1.f - color.a) + color;
    }

    if (mRenderTarget) {
        mRenderTarget->set(glm::uvec2(pos), AColor(glm::clamp(combined, 0.f, 1.f)));
    } else {
        mContext->putPixel(glm::uvec2(pos), glm::u8vec4(glm::clamp(combined, 0.f, 1.f) * 255.f));
    }
}

void SoftwareRenderer::solidRectangles(const ADisplayList::SolidRectangles& v, const glm::mat4& transform, const APaint& paint) {
    for (const auto& inst : v.instances) {
        auto p1 = transform * glm::vec4(inst.position, 0.f, 1.f);
        auto p2 = transform * glm::vec4(inst.position + inst.size, 0.f, 1.f);
        AColor color = inst.color.premultiply();
        for (int y = (int)p1.y; y < (int)p2.y; ++y) {
            for (int x = (int)p1.x; x < (int)p2.x; ++x) {
                putPixel({x, y}, color, paint);
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
void SoftwareRenderer::gradientRectangles(const ADisplayList::GradientRectangles& v, const glm::mat4& transform, const APaint& paint) {
    if (v.instances.empty()) return;
    ALinearGradientBrush brush {
        v.colors,
        v.rotation
    };
    aui::render::brush::gradient::Helper helper(brush);
    glm::vec4 c1 = v.colors.size() > 0 ? v.colors[0].color.premultiply() : glm::vec4(0.f);
    glm::vec4 c2 = v.colors.size() > 1 ? v.colors[1].color.premultiply() : (v.colors.size() > 0 ? v.colors[0].color.premultiply() : glm::vec4(0.f));

    for (const auto& inst : v.instances) {
        auto p1 = transform * glm::vec4(inst.position, 0.f, 1.f);
        auto p2 = transform * glm::vec4(inst.position + inst.size, 0.f, 1.f);
        float width = p2.x - p1.x;
        float height = p2.y - p1.y;

        for (int y = (int)p1.y; y < (int)p2.y; ++y) {
            for (int x = (int)p1.x; x < (int)p2.x; ++x) {
                float uvX = (x - p1.x) / width;
                float uvY = (y - p1.y) / height;
                glm::vec2 uv(uvX, uvY);
                glm::vec3 transformedUv = helper.matrix * glm::vec3(uv, 1.f);
                float t = glm::clamp(transformedUv.x, 0.f, 1.f);
                AColor color = glm::mix(c1, c2, t) * inst.color.premultiply();
                putPixel({x, y}, color, paint);
            }
        }
    }
}
void SoftwareRenderer::texturedRectangles(const ADisplayList::TexturedRectangles& v, const glm::mat4& transform, const APaint& paint) {
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
                putPixel({x, y}, AColor(texColor * inst.color).premultiply(), paint);
            }
        }
    }
}
void SoftwareRenderer::solidRoundedRectangles(const ADisplayList::SolidRoundedRectangles& v, const glm::mat4& transform, const APaint& paint) {
    for (const auto& inst : v.instances) {
        auto p1 = transform * glm::vec4(inst.position, 0.f, 1.f);
        auto p2 = transform * glm::vec4(inst.position + inst.size, 0.f, 1.f);
        AColor color = inst.color.premultiply();
        float radius = v.radius;
        float maxRadius = std::min(inst.size.x, inst.size.y) * 0.5f;
        if (radius > maxRadius) {
            radius = maxRadius;
        }
        glm::mat4 invTransform = glm::inverse(transform);
        float scale = (p2.x - p1.x) / inst.size.x;

        for (int y = (int)p1.y; y < (int)p2.y; ++y) {
            for (int x = (int)p1.x; x < (int)p2.x; ++x) {
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
void SoftwareRenderer::gradientRoundedRectangles(const ADisplayList::GradientRoundedRectangles& v, const glm::mat4& transform, const APaint& paint) {
    if (v.instances.empty()) return;
    ALinearGradientBrush brush {
        v.colors,
        v.rotation
    };
    aui::render::brush::gradient::Helper helper(brush);
    glm::vec4 c1 = v.colors.size() > 0 ? v.colors[0].color.premultiply() : glm::vec4(0.f);
    glm::vec4 c2 = v.colors.size() > 1 ? v.colors[1].color.premultiply() : (v.colors.size() > 0 ? v.colors[0].color.premultiply() : glm::vec4(0.f));

    for (const auto& inst : v.instances) {
        auto p1 = transform * glm::vec4(inst.position, 0.f, 1.f);
        auto p2 = transform * glm::vec4(inst.position + inst.size, 0.f, 1.f);
        AColor color = inst.color.premultiply();
        float radius = v.radius;
        float maxRadius = std::min(inst.size.x, inst.size.y) * 0.5f;
        if (radius > maxRadius) {
            radius = maxRadius;
        }
        glm::mat4 invTransform = glm::inverse(transform);
        float scale = (p2.x - p1.x) / inst.size.x;
        float width = p2.x - p1.x;
        float height = p2.y - p1.y;

        for (int y = (int)p1.y; y < (int)p2.y; ++y) {
            for (int x = (int)p1.x; x < (int)p2.x; ++x) {
                glm::vec4 localPos4 = invTransform * glm::vec4((float)x + 0.5f, (float)y + 0.5f, 0.f, 1.f);
                glm::vec2 localPos = glm::vec2(localPos4.x, localPos4.y) - inst.position;
                float a = roundedRectCoverage(localPos, inst.size, radius, scale);
                if (a > 0.001f) {
                    float uvX = (x - p1.x) / width;
                    float uvY = (y - p1.y) / height;
                    glm::vec2 uv(uvX, uvY);
                    glm::vec3 transformedUv = helper.matrix * glm::vec3(uv, 1.f);
                    float t = glm::clamp(transformedUv.x, 0.f, 1.f);
                    AColor gradColor = glm::mix(c1, c2, t);
                    putPixel({x, y}, color * gradColor * a, paint);
                }
            }
        }
    }
}
void SoftwareRenderer::texturedRoundedRectangles(const ADisplayList::TexturedRoundedRectangles& v, const glm::mat4& transform, const APaint& paint) {
    auto texture = dynamic_cast<SoftwareTexture*>(v.texture.get());
    if (!texture) return;
    const auto& img = texture->getImage();
    if (img.width() == 0 || img.height() == 0) return;

    for (const auto& inst : v.instances) {
        auto p1 = transform * glm::vec4(inst.position, 0.f, 1.f);
        auto p2 = transform * glm::vec4(inst.position + inst.size, 0.f, 1.f);
        AColor color = inst.color.premultiply();
        float radius = v.radius;
        float maxRadius = std::min(inst.size.x, inst.size.y) * 0.5f;
        if (radius > maxRadius) {
            radius = maxRadius;
        }
        glm::mat4 invTransform = glm::inverse(transform);
        float scale = (p2.x - p1.x) / inst.size.x;
        float width = p2.x - p1.x;
        float height = p2.y - p1.y;

        for (int y = (int)p1.y; y < (int)p2.y; ++y) {
            for (int x = (int)p1.x; x < (int)p2.x; ++x) {
                glm::vec4 localPos4 = invTransform * glm::vec4((float)x + 0.5f, (float)y + 0.5f, 0.f, 1.f);
                glm::vec2 localPos = glm::vec2(localPos4.x, localPos4.y) - inst.position;
                float a = roundedRectCoverage(localPos, inst.size, radius, scale);
                if (a > 0.001f) {
                    float uvX = (x - p1.x) / width;
                    float uvY = (y - p1.y) / height;
                    glm::uvec2 texPos((unsigned)(uvX * glm::max(0.f, (float)img.width() - 1.f)),
                                      (unsigned)(uvY * glm::max(0.f, (float)img.height() - 1.f)));
                    AColor texColor;
                    if (texPos.x < img.width() && texPos.y < img.height()) {
                        texColor = img.get(texPos);
                    } else {
                        texColor = AColor::RED;
                    }
                    texColor = texColor.premultiply();
                    putPixel({x, y}, texColor * color * a, paint);
                }
            }
        }
    }
}
void SoftwareRenderer::rectangleBorders(const ADisplayList::RectangleBorders& v, const glm::mat4& transform, const APaint& paint) {
    for (const auto& inst : v.instances) {
        AColor color = inst.color.premultiply();
        auto drawSubRect = [&](glm::vec2 pos, glm::vec2 size) {
            auto p1 = transform * glm::vec4(pos, 0.f, 1.f);
            auto p2 = transform * glm::vec4(pos + size, 0.f, 1.f);
            for (int y = (int)p1.y; y < (int)p2.y; ++y) {
                for (int x = (int)p1.x; x < (int)p2.x; ++x) {
                    putPixel({x, y}, color, paint);
                }
            }
        };
        drawSubRect(inst.position, {inst.size.x, v.lineWidth});
        drawSubRect({inst.position.x, inst.position.y + inst.size.y - v.lineWidth}, {inst.size.x, v.lineWidth});
        drawSubRect({inst.position.x, inst.position.y + v.lineWidth}, {v.lineWidth, inst.size.y - v.lineWidth * 2.f});
        drawSubRect({inst.position.x + inst.size.x - v.lineWidth, inst.position.y + v.lineWidth}, {v.lineWidth, inst.size.y - v.lineWidth * 2.f});
    }
}
void SoftwareRenderer::roundedRectangleBorders(const ADisplayList::RoundedRectangleBorders& v, const glm::mat4& transform, const APaint& paint) {
    for (const auto& inst : v.instances) {
        AColor color = inst.color.premultiply();
        float radius = v.radius;
        float maxRadius = std::min(inst.size.x, inst.size.y) * 0.5f;
        if (radius > maxRadius) {
            radius = maxRadius;
        }
        auto p1 = transform * glm::vec4(inst.position, 0.f, 1.f);
        auto p2 = transform * glm::vec4(inst.position + inst.size, 0.f, 1.f);
        glm::mat4 invTransform = glm::inverse(transform);
        float scale = (p2.x - p1.x) / inst.size.x;

        for (int y = (int)p1.y; y < (int)p2.y; ++y) {
            for (int x = (int)p1.x; x < (int)p2.x; ++x) {
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
void SoftwareRenderer::boxShadow(const ADisplayList::BoxShadow& v, const glm::mat4& transform, const APaint& paint) {
    float sigma = v.blurRadius / 2.f;
    float padding = v.blurRadius * 2.f;
    glm::vec2 pos = v.position - padding;
    glm::vec2 size = v.size + padding * 2.f;

    auto p1 = transform * glm::vec4(pos, 0.f, 1.f);
    auto p2 = transform * glm::vec4(pos + size, 0.f, 1.f);
    glm::mat4 invTransform = glm::inverse(transform);
    AColor color = v.color.premultiply();

    for (int y = (int)p1.y; y < (int)p2.y; ++y) {
        for (int x = (int)p1.x; x < (int)p2.x; ++x) {
            glm::vec4 localPos = invTransform * glm::vec4((float)x + 0.5f, (float)y + 0.5f, 0.f, 1.f);
            glm::vec2 val = glm::vec2(localPos.x, localPos.y);
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
void SoftwareRenderer::boxShadowInner(const ADisplayList::BoxShadowInner& v, const glm::mat4& transform, const APaint& paint) {
    float sigma = v.blurRadius / 2.f;
    glm::vec2 lower = v.position + v.offset + glm::vec2(v.spreadRadius);
    glm::vec2 upper = v.position + v.size + v.offset - glm::vec2(v.spreadRadius);

    auto p1 = transform * glm::vec4(v.position, 0.f, 1.f);
    auto p2 = transform * glm::vec4(v.position + v.size, 0.f, 1.f);
    glm::mat4 invTransform = glm::inverse(transform);
    float scale = (p2.x - p1.x) / v.size.x;
    AColor color = v.color.premultiply();

    for (int y = (int)p1.y; y < (int)p2.y; ++y) {
        for (int x = (int)p1.x; x < (int)p2.x; ++x) {
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
void SoftwareRenderer::glyphs(const ADisplayList::Glyphs& v, const glm::mat4& transform, const APaint& paint) {
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
                float u = ((float)x + 0.5f - p1.x) / width;
                float v_uv = ((float)y + 0.5f - p1.y) / height;

                float tx = glm::mix(inst.u1.x, inst.u2.x, u) * (float)img.width();
                float ty = glm::mix(inst.u1.y, inst.u2.y, v_uv) * (float)img.height();

                int texX = glm::clamp((int)glm::floor(tx), 0, (int)img.width() - 1);
                int texY = glm::clamp((int)glm::floor(ty), 0, (int)img.height() - 1);

                AColor maskColor = img.get(glm::uvec2(texX, texY));
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

                    glm::vec4 dst;
                    if (mRenderTarget) {
                        dst = glm::vec4(mRenderTarget->get(glm::uvec2(x, y))) / 255.f;
                    } else {
                        dst = glm::vec4(mContext->getPixel(glm::uvec2(x, y))) / 255.f;
                    }

                    AColor res;
                    res.r = dst.r * (1.f - maskColor.r) + pColor.r * maskColor.r;
                    res.g = dst.g * (1.f - maskColor.g) + pColor.g * maskColor.g;
                    res.b = dst.b * (1.f - maskColor.b) + pColor.b * maskColor.b;
                    float avgMask = (maskColor.r + maskColor.g + maskColor.b) / 3.f;
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
void SoftwareRenderer::lines(const ADisplayList::Lines& v, const glm::mat4& transform, const APaint& paint) {
    if (v.points.size() < 2) return;
    float widthPx = v.width.getValuePx();
    AColor color = paint.color.premultiply();

    for (size_t i = 1; i < v.points.size(); ++i) {
        auto p0 = transform * glm::vec4(v.points[i-1], 0.f, 1.f);
        auto p1 = transform * glm::vec4(v.points[i], 0.f, 1.f);
        drawLine({(int)p0.x, (int)p0.y}, {(int)p1.x, (int)p1.y}, widthPx, color, paint);
    }
}
void SoftwareRenderer::points(const ADisplayList::Points& v, const glm::mat4& transform, const APaint& paint) {
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
void SoftwareRenderer::lines(const ADisplayList::LineBatches& v, const glm::mat4& transform, const APaint& paint) {
    if (v.points.empty()) return;
    float widthPx = v.width.getValuePx();
    AColor color = paint.color.premultiply();

    for (const auto& [pt1, pt2] : v.points) {
        auto p0 = transform * glm::vec4(pt1, 0.f, 1.f);
        auto p1 = transform * glm::vec4(pt2, 0.f, 1.f);
        drawLine({(int)p0.x, (int)p0.y}, {(int)p1.x, (int)p1.y}, widthPx, color, paint);
    }
}
void SoftwareRenderer::squareSector(const ADisplayList::SquareSector& v, const glm::mat4& transform, const APaint& paint) {
    auto p1 = transform * glm::vec4(v.position, 0.f, 1.f);
    auto p2 = transform * glm::vec4(v.position + v.size, 0.f, 1.f);
    AColor color = paint.color.premultiply();
    float width = p2.x - p1.x;
    float height = p2.y - p1.y;

    float beginRad = v.begin.radians();
    float endRad = v.end.radians();
    if (beginRad < 0.f) beginRad += 2.f * glm::pi<float>();
    if (endRad < 0.f) endRad += 2.f * glm::pi<float>();

    for (int y = (int)p1.y; y < (int)p2.y; ++y) {
        for (int x = (int)p1.x; x < (int)p2.x; ++x) {
            float uvX = (x - p1.x) / width;
            float uvY = (y - p1.y) / height;
            glm::vec2 uv = glm::vec2(uvX, uvY) * 2.f - glm::vec2(1.f);
            float angle = glm::atan(uv.y, uv.x);
            if (angle < 0.f) angle += 2.f * glm::pi<float>();
            if (angle >= beginRad && angle <= endRad) {
                putPixel({x, y}, color, paint);
            }
        }
    }
}
void SoftwareRenderer::backdrops(const ADisplayList::Backdrop& v, const glm::mat4& transform) {}
void SoftwareRenderer::backdrops(glm::ivec2 fbSize, glm::ivec2 size, std::span<const ass::Backdrop::Preprocessed> backdrops) {}

glm::mat4 SoftwareRenderer::getProjectionMatrix() const { return glm::mat4(1.0f); }

_<ITexture> SoftwareRenderer::createTexture(glm::u32vec2 size, APixelFormat format, TextureFilter filter) {
    auto t = _new<SoftwareTexture>();
    t->upload(AImage(size, format));
    return t;
}

