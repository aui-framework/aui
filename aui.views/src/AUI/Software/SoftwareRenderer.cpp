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

//
// Created by Alex2772 on 12/5/2021.
//

#include <range/v3/view.hpp>
#include <AUI/Traits/callables.h>
#include "SoftwareRenderer.h"
#include "AUI/Enum/ImageRendering.h"
#include "AUI/SL/SL.h"
#include "SoftwareTexture.h"
#include "AUI/Render/Brush/Gradient.h"
#include "glm/vector_relational.hpp"
#include <AUISL/Generated/rect_solid.fsh.software.h>
#include <AUISL/Generated/shadow.fsh.software.h>
#include <AUISL/Generated/rect_gradient.fsh.software.h>

struct BrushHelper {
    SoftwareRenderer* renderer;
    int &x, &y;
    glm::vec2 end;
    glm::vec2 position;

    BrushHelper(SoftwareRenderer* renderer,
                int& x,
                int& y,
                glm::ivec2& end,
                glm::ivec2& position) : renderer(renderer), x(x), y(y), end(end), position(position) {}

    void operator()(const ASolidBrush& brush) noexcept {
        using namespace aui::sl_gen::rect_solid::fsh::software;

        renderer->putPixel({x, y}, Shader::entry({}, Shader::Uniform {
                .color = renderer->getColor() * brush.solidColor
        }).albedo);
    }

    void operator()(const ATexturedBrush& brush) noexcept {
        if (!textureHelper) {
            auto tex = dynamic_cast<SoftwareTexture*>(brush.texture.get());
            textureHelper = {
                brush.uv1 || brush.uv2 || glm::ivec2(end - position) != glm::ivec2(tex->getImage().size()),
                tex
            };
        }
        if (textureHelper->slowMethod) {
            // slower method
            auto rawPixelPos = glm::vec2{x, y};
            auto surfaceUvCoords = (rawPixelPos - position) / (end - position);
            auto uv1 = brush.uv1.valueOr(glm::ivec2{0, 0});
            auto uv2 = brush.uv2.valueOr(glm::ivec2{0, 0});
            auto uv = glm::vec2{ glm::mix(uv1.x, uv2.x, surfaceUvCoords.x), glm::mix(uv1.y, uv2.y, surfaceUvCoords.y) };
            const auto& image = textureHelper->texture->getImage();
            auto imagePixelCoords = glm::ivec2{glm::vec2(image.size()) * uv};
            if (glm::any(glm::lessThan(imagePixelCoords, glm::ivec2(0)))) {
                return;
            }

            auto color = image.get({imagePixelCoords.x, imagePixelCoords.y});
            renderer->putPixel({ x, y }, renderer->getColor() * color);
        } else {
            // faster method
            auto color = textureHelper->texture->getImage().get(glm::uvec2{ x, y } - glm::uvec2(position));
            renderer->putPixel({ x, y }, renderer->getColor() * color);
        }
    }


    void operator()(const ALinearGradientBrush& brush) noexcept {
        using namespace aui::sl_gen::rect_gradient::fsh::software;

        aui::render::brush::gradient::Helper h(brush);
        const auto output = Shader::entry({.uv = calculateUv()},
                                          {
                                            //.gradientMap = aui::sl_gen::Texture2D(h.gradientMap(), ImageRendering::SMOOTH),
                                            .color1 = h.colors[0],
                                            .color2 = h.colors[1],
                                            .matUv = h.matrix,
                                            .color = renderer->getColor()
                                          });
        renderer->putPixel({ x, y }, output.albedo);
    }

    void operator()(const ACustomShaderBrush& brush) noexcept {
    }

private:
    struct TextureHelper {
        bool slowMethod;
        SoftwareTexture* texture;
    };
    AOptional<TextureHelper> textureHelper;

    [[nodiscard]]
    glm::vec2 calculateUv() const noexcept {
        return (glm::vec2(x, y) - position) / (end - position);
    }
};

struct RoundedRect {
    int radius;
    int radius2;
    glm::ivec2 size;
    glm::ivec2 halfSize;
    glm::ivec2 transformedPosition;
    glm::ivec2 center;

    RoundedRect(int radius, const glm::ivec2& size, const glm::ivec2& transformedPosition):
        radius(radius),
        radius2(radius * radius),
        size(size),
        halfSize(size / 2),
        transformedPosition(transformedPosition),
        center(transformedPosition + halfSize)
    {

    }

    template<bool isAntialiasing>
    int test(const glm::ivec2& absed) const {
        int radius25 = radius2 * 25; // for antialiasing
        if (absed.x + radius >= halfSize.x && absed.y + radius >= halfSize.y) {
            auto circleCoordSpace = halfSize - absed - radius;
            // chamfer check
            if (circleCoordSpace.x + circleCoordSpace.y + radius < 0) {
                // circle check
                if constexpr (isAntialiasing) {
                    circleCoordSpace *= 5;
                    int accumulator = 0;
                    for (int oY = 0; oY < 5; ++oY) {
                        for (int oX = 0; oX < 5; ++oX) {
                            int fX = circleCoordSpace.x + oX;
                            int fY = circleCoordSpace.y + oY;
                            if (fX * fX + fY * fY < radius25) {
                                accumulator += 1;
                            }
                        }
                    }
                    return accumulator;
                }
                else {
                    return circleCoordSpace.x * circleCoordSpace.x + circleCoordSpace.y * circleCoordSpace.y > radius2;
                }
            }
        }
        return 25;
    }

    glm::ivec2 abs(const glm::ivec2& pos) {
        auto absed = pos - center;
        if (absed.x < 0) {
            absed.x = -absed.x;
        }
        else {
            absed.x += !(size.x % 2);
        }
        if (absed.y < 0) {
            absed.y = -absed.y;
        }
        else {
            absed.y += !(size.y % 2);
        }
        return absed;
    }
};


glm::mat4 SoftwareRenderer::getProjectionMatrix() const {
    return glm::mat4(1.f);
}

void SoftwareRenderer::rectangle(const ABrush& brush,
                                 glm::vec2 position,
                                 glm::vec2 size) {
    auto transformedPosition = glm::ivec2(mTransform * glm::vec4(position, 1.f, 1.f));
    auto end = transformedPosition + glm::ivec2(size);

    int x, y;

    auto sw = BrushHelper(this, x, y, end, transformedPosition);

    for (y = transformedPosition.y; y < end.y; ++y) {
        for (x = transformedPosition.x; x < end.x; ++x) {
            std::visit(sw, brush);
        }
    }
}

void SoftwareRenderer::roundedRectangle(const ABrush& brush,
                                        glm::vec2 position,
                                        glm::vec2 size,
                                        float radius) {
    RoundedRect r(int(radius), glm::ivec2(size), glm::ivec2(mTransform * glm::vec4(position, 1.f, 1.f)));
    auto end = r.transformedPosition + r.size;

    int x, y;

    auto sw = BrushHelper(this, x, y, end, r.transformedPosition);

    for (y = r.transformedPosition.y; y < end.y; ++y) {
        for (x = r.transformedPosition.x; x < end.x; ++x) {
            if (int accumulator = r.test<true>(r.abs({x, y})); accumulator != 0) {
                float alphaCopy = mColor.a;
                mColor.a *= accumulator;
                mColor.a /= 25;
                std::visit(sw, brush);
                mColor.a = alphaCopy;
            }
        }
    }
}

void SoftwareRenderer::rectangleBorder(const ABrush& brush,
                                       glm::vec2 position,
                                       glm::vec2 size,
                                       float lineWidth) {
    rectangle(brush, position, {size.x, lineWidth});
    rectangle(brush, position + glm::vec2{0, size.y - lineWidth}, {size.x, lineWidth});
    rectangle(brush, position + glm::vec2{0, lineWidth}, {lineWidth, size.y - 2 * lineWidth});
    rectangle(brush, position + glm::vec2{size.x - lineWidth, lineWidth}, {lineWidth, size.y - 2 * lineWidth});
}

void SoftwareRenderer::roundedRectangleBorder(const ABrush& brush,
                                              glm::vec2 position,
                                              glm::vec2 size,
                                              float radius,
                                              int borderWidth) {
    auto pos = glm::ivec2(mTransform * glm::vec4(position, 1.f, 1.f));
    RoundedRect outside(int(radius), glm::ivec2(size), pos);
    RoundedRect inside(int(radius) - borderWidth, glm::ivec2(size) - glm::ivec2(borderWidth * 2), pos + glm::ivec2(borderWidth));
    auto end = outside.transformedPosition + outside.size;

    int x, y;

    auto sw = BrushHelper(this, x, y, end, outside.transformedPosition);

    for (y = outside.transformedPosition.y; y < end.y; ++y) {
        for (x = outside.transformedPosition.x; x < end.x; ++x) {
            int accumulator = outside.test<true>(outside.abs({ x, y }));

            if (x - outside.transformedPosition.x >= borderWidth &&
                y - outside.transformedPosition.y >= borderWidth) {
                if (x < end.x - borderWidth &&
                    y < end.y - borderWidth) {
                    accumulator -= inside.test<true>(inside.abs({ x, y }));
                }
            }

            if (accumulator != 0) {
                float alphaCopy = mColor.a;
                mColor.a *= accumulator;
                mColor.a /= 25;
                std::visit(sw, brush);
                mColor.a = alphaCopy;
            }
        }
    }
}

void SoftwareRenderer::boxShadow(glm::vec2 position,
                                 glm::vec2 size,
                                 float blurRadius,
                                 const AColor& color) {

    auto transformedPos = glm::vec2(mTransform * glm::vec4(position, 1.f, 1.f));

    //transformedPos -= blurRadius;
    glm::ivec2 iTransformedPos(transformedPos - blurRadius);
    auto iSize = glm::ivec2(size + blurRadius * 2.f);


    using namespace aui::sl_gen::shadow::fsh::software;
    const Shader::Uniform uniform{
        .color = mColor * color,
        .lower = transformedPos + size,
        .upper = transformedPos,
        .sigma = blurRadius / 2.f,
    };

    for (int y = 0; y < iSize.y; ++y) { 
        for (int x = 0; x < iSize.x; ++x) {
            const auto result = Shader::entry(Shader::Inter {
                .vertex = glm::ivec4(iTransformedPos + glm::ivec2{x, y}, 0, 1),
            }, uniform).albedo;

            /*
            glm::vec4 query = glm::vec4(pass_uv - glm::vec2(lower), pass_uv - glm::vec2(upper));
            glm::vec4 integral = 0.5f + 0.5f * erf(query * (glm::sqrt(0.5f) / sigma));
            float alpha = glm::clamp((integral.z - integral.x) * (integral.w - integral.y), 0.0f, 1.0f);
*/
            putPixel(iTransformedPos + glm::ivec2{ x, y }, result);
        }
    }
}
void SoftwareRenderer::boxShadowInner(glm::vec2 position,
                                      glm::vec2 size,
                                      float blurRadius,
                                      float spreadRadius,
                                      float borderRadius,
                                      const AColor& color,
                                      glm::vec2 offset) {

    auto transformedPos = glm::vec2(mTransform * glm::vec4(position, 1.f, 1.f));

    //transformedPos -= blurRadius;
    glm::ivec2 iTransformedPos(transformedPos);
    auto iSize = glm::ivec2(size + blurRadius * 2.f);


    using namespace aui::sl_gen::shadow::fsh::software;
    const Shader::Uniform uniform{
        .color = mColor * color,
        .lower = size,
        .upper = transformedPos,
        .sigma = blurRadius / 2.f,
    };

    for (int y = 0; y < iSize.y; ++y) { 
        for (int x = 0; x < iSize.x; ++x) {
            const auto result = Shader::entry(Shader::Inter {
                .vertex = glm::vec4(transformedPos + glm::vec2{x, y}, 0.f, 1.f),
            }, uniform).albedo;

            /*
            glm::vec4 query = glm::vec4(pass_uv - glm::vec2(lower), pass_uv - glm::vec2(upper));
            glm::vec4 integral = 0.5f + 0.5f * erf(query * (glm::sqrt(0.5f) / sigma));
            float alpha = glm::clamp((integral.z - integral.x) * (integral.w - integral.y), 0.0f, 1.0f);
*/
            putPixel(iTransformedPos + glm::ivec2{ x, y }, result);
        }
    }
}


void SoftwareRenderer::setBlending(Blending blending) {
    mBlending = blending;
}

void SoftwareRenderer::pushMaskBefore() {
    mDrawingToStencil = true;
    mDrawingStencilDirection = INCREASE;
}

void SoftwareRenderer::pushMaskAfter() {
    mDrawingToStencil = false;
    mStencilDepth += 1;
}

void SoftwareRenderer::popMaskBefore() {
    mDrawingToStencil = true;
    mDrawingStencilDirection = DECREASE;
}

void SoftwareRenderer::popMaskAfter() {
    mDrawingToStencil = false;
    mStencilDepth -= 1;
}

struct CharEntry {
    glm::ivec2 position;
    AImage* image;
};

class SoftwarePrerenderedString: public IRenderer::IPrerenderedString {
private:
    SoftwareRenderer* mRenderer;
    AVector<CharEntry> mCharEntries;
    int mWidth = 0;
    int mHeight = 0;
    FontRendering mFontRendering;

public:
    SoftwarePrerenderedString(SoftwareRenderer* renderer,
                              AVector<CharEntry> charEntries,
                              int width,
                              int height,
                              FontRendering fontRendering) : mRenderer(renderer),
                                                             mCharEntries(std::move(charEntries)),
                                                             mWidth(width),
                                                             mHeight(height),
                                                             mFontRendering(fontRendering) {}

    void draw() override {
        auto finalColor = AColor(mRenderer->getColor());
        if (finalColor.isFullyTransparent()) return;
        switch (mFontRendering) {
            case FontRendering::SUBPIXEL:
                for (const auto& entry : mCharEntries) {
                    auto transformedPosition = glm::ivec2(mRenderer->getTransform() * glm::vec4(entry.position, 1.f, 1.f));
                    auto size = entry.image->size();
                    for (int y = 0; y < size.y; ++y) {
                        for (int x = 0; x < size.x; ++x) {
                            auto color = entry.image->get({x, y});
                            
                            mRenderer->putPixel(transformedPosition + glm::ivec2{ x, y }, AColor{ color.r, color.g, color.b, color.a * finalColor.a }, Blending::INVERSE_SRC);
                            mRenderer->putPixel(transformedPosition + glm::ivec2{ x, y }, color * finalColor, Blending::ADDITIVE);
                        }
                    }
                }
                break;
            case FontRendering::ANTIALIASING:
                for (const auto& entry : mCharEntries) {
                    auto transformedPosition = glm::ivec2(mRenderer->getTransform() * glm::vec4(entry.position, 1.f, 1.f));
                    auto size = entry.image->size();
                    for (int y = 0; y < size.y; ++y) {
                        for (int x = 0; x < size.x; ++x) {
                            mRenderer->putPixel(transformedPosition + glm::ivec2{ x, y }, { finalColor.r, finalColor.g, finalColor.b, finalColor.a * entry.image->get({x, y}).r });
                        }
                    }
                }
                break;
            case FontRendering::NEAREST:
                break;
        }
    }

    int getWidth() override {
        return mWidth;
    }

    int getHeight() override {
        return mHeight;
    }
};

class SoftwareMultiStringCanvas: public IRenderer::IMultiStringCanvas {
private:
    SoftwareRenderer* mRenderer;
    AFontStyle mFontStyle;
    int mAdvanceX = 0;
    int mAdvanceY = 0;
    AVector<CharEntry> mCharEntries;

public:
    SoftwareMultiStringCanvas(SoftwareRenderer* renderer, const AFontStyle& fontStyle) : mRenderer(renderer),
                                                                                         mFontStyle(fontStyle) {}

    void addString(const glm::ivec2& position,
                   const AString& text) noexcept override {
        mCharEntries.reserve(mCharEntries.capacity() + text.length());
        auto& font = mFontStyle.font;
        auto fe = mFontStyle.getFontEntry();

        const bool hasKerning = font->isHasKerning();

        int prevWidth = -1;

        int advanceX = position.x;
        int advanceY = position.y - mFontStyle.font->getDescenderHeight(mFontStyle.size);
        size_t counter = 0;
        int advance = advanceX;
        for (auto i = text.begin(); i != text.end(); ++i, ++counter) {
            wchar_t c = *i;
            if (c == ' ') {
                notifySymbolAdded({glm::ivec2{advance, advanceY}});
                advance += mFontStyle.getSpaceWidth();
            }
            else if (c == '\n') {
                notifySymbolAdded({glm::ivec2{advance, advanceY}});
                advanceX = (glm::max)(advanceX, advance);
                advance = position.x;
                advanceY += mFontStyle.getLineHeight();
                nextLine();
            }
            else {
                AFont::Character& ch = font->getCharacter(fe, c);
                if (ch.empty()) {
                    advance += mFontStyle.getSpaceWidth();
                    continue;
                }
                if ((advance >= 0 && advance <= 99999) /* || gui3d */) {
                    glm::ivec2 pos{ advance,  advanceY };
                    pos += ch.horizontal.bearing;
                    notifySymbolAdded({pos});
                    mCharEntries.push_back(CharEntry{
                            pos,
                            ch.image.get()
                    });
                }

                if (hasKerning) {
                    auto next = std::next(i);
                    if (next != text.end())
                    {
                        auto kerning = font->getKerning(c, *next);
                        advance += kerning.x;
                    }
                }

                advance += ch.horizontal.advance;
                advance = glm::floor(advance);
            }
        }

        notifySymbolAdded({glm::ivec2{advance, advanceY}});

        mAdvanceX = (glm::max)(mAdvanceX, (glm::max)(advanceX, advance));
        mAdvanceY = advanceY + mFontStyle.getLineHeight();
    }

    _<IRenderer::IPrerenderedString> finalize() noexcept override {
        return _new<SoftwarePrerenderedString>(mRenderer,
                                               std::move(mCharEntries),
                                               mAdvanceX,
                                               mAdvanceY,
                                               mFontStyle.fontRendering);
    }
};


void SoftwareRenderer::string(glm::vec2 position,
                              const AString& string,
                              const AFontStyle& fs) {
    SoftwareMultiStringCanvas c(this, fs);
    c.addString(position, string);
    c.finalize()->draw();
}

_<IRenderer::IPrerenderedString> SoftwareRenderer::prerenderString(glm::vec2 position,
                                                                   const AString& text,
                                                                   const AFontStyle& fs) {
    if (text.empty()) return nullptr;

    SoftwareMultiStringCanvas c(this, fs);
    c.addString(position, text);

    return c.finalize();
}

_unique<ITexture> SoftwareRenderer::createNewTexture() {
    return std::make_unique<SoftwareTexture>();
}

_<IRenderer::IMultiStringCanvas> SoftwareRenderer::newMultiStringCanvas(const AFontStyle& style) {
    return _new<SoftwareMultiStringCanvas>(this, style);
}

void SoftwareRenderer::setWindow(AWindowBase* window) {
    IRenderer::setWindow(window);
    if (auto context = dynamic_cast<SoftwareRenderingContext*>(window->getRenderingContext().get())) {
        mContext = context;
    } else {
        mContext = nullptr;
    }
}

void SoftwareRenderer::drawLine(const ABrush& brush, glm::vec2 p1, glm::vec2 p2, const ABorderStyle& style, AMetric width) {
    // TODO
    if (p1.x == p2.x || p1.y == p2.y) {
        auto begin = glm::min(p1, p2);
        rectangle(brush, begin, glm::max(p1, p2) - begin + glm::vec2(1));
        return;
    }
}

void SoftwareRenderer::lines(const ABrush& brush, AArrayView<glm::vec2> points, const ABorderStyle& style, AMetric width) {
    if (points.size() == 0) {
        return;
    }

    auto prevPoint = points[0];
    for (auto point : points | ranges::views::drop(1)) {
        drawLine(brush, prevPoint, point, style, width);
        prevPoint = point;
    }
}

void SoftwareRenderer::lines(const ABrush& brush, AArrayView<std::pair<glm::vec2, glm::vec2>> points, const ABorderStyle& style, AMetric width) {
    for (auto[p1, p2] : points) {
        drawLine(brush, p1, p2, style, width);
    }
}

void SoftwareRenderer::points(const ABrush& brush, AArrayView<glm::vec2> points, AMetric size) {
    if (points.size() == 0) {
        return;
    }
}

void SoftwareRenderer::squareSector(const ABrush& brush, const glm::vec2& position, const glm::vec2& size,
                                    AAngleRadians begin, AAngleRadians end) {}

_unique<IRenderViewToTexture> SoftwareRenderer::newRenderViewToTexture() noexcept {
    return nullptr;
}
