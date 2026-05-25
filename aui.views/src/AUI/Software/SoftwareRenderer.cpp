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

// ---------------------------------------------------------------------------------------------------------------------
// Text rendering implementations
// ---------------------------------------------------------------------------------------------------------------------

struct CharacterGlyph {
    glm::vec2 position;
    glm::vec2 size;
    glm::vec2 u1;
    glm::vec2 u2;
    AColor color;
};

class SoftwarePrerenderedString : public IRenderer::IPrerenderedString {
private:
    AVector<CharacterGlyph> mGlyphs;
    int mTextWidth;
    int mTextHeight;

public:
    SoftwarePrerenderedString(AVector<CharacterGlyph> glyphs,
                            int textWidth,
                            int textHeight) :
        mGlyphs(std::move(glyphs)),
        mTextWidth(textWidth),
        mTextHeight(textHeight) {}

    void draw(ACanvas& canvas) override {
        for (const auto& g : mGlyphs) {
            canvas.glyphRect(nullptr, g.position, g.size, g.u1, g.u2, g.color);
        }
    }

    int getWidth() override { return mTextWidth; }
    int getHeight() override { return mTextHeight; }
};

class SoftwareMultiStringCanvas : public IRenderer::IMultiStringCanvas {
private:
    AVector<CharacterGlyph> mGlyphs;
    AFontStyle mFontStyle;
    int mAdvanceX = 0;
    int mAdvanceY = 0;

public:
    SoftwareMultiStringCanvas(const AFontStyle& fontStyle) :
        mFontStyle(fontStyle) {
    }

    template<class UnicodeString>
    void addStringT(const glm::ivec2& position, UnicodeString text) noexcept {
        auto& font = mFontStyle.font;
        auto fe = mFontStyle.getFontEntry();

        const bool hasKerning = font->isHasKerning();

        int advanceX = position.x;
        int advanceY = position.y;
        float advance = (float)advanceX;
        for (auto i = text.begin(); i != text.end(); ++i) {
            AChar c = *i;
            if (c == ' ') {
                advance += mFontStyle.getSpaceWidth();
            } else if (c == '\n') {
                advanceX = (glm::max)(advanceX, int(glm::ceil(advance)));
                advance = (float)position.x;
                advanceY += mFontStyle.getLineHeight();
            } else {
                AFont::Character& ch = font->getCharacter(fe, c);
                if (ch.empty()) {
                    advance += mFontStyle.getSpaceWidth();
                    continue;
                }
                
                int posX = (int)advance + ch.horizontal.bearing.x;
                int posY = advanceY - ch.horizontal.bearing.y;
                int width = ch.image->width();
                int height = ch.image->height();

                mGlyphs.push_back({
                    glm::vec2(posX, posY),
                    glm::vec2(width, height),
                    glm::vec2(0.f, 0.f),
                    glm::vec2(1.f, 1.f),
                    AColor::WHITE
                });

                if (hasKerning) {
                    auto next = std::next(i);
                    if (next != text.end()) {
                        auto kerning = font->getKerning(c, *next);
                        advance += (float)kerning.x;
                    }
                }
                advance += (float)ch.horizontal.advance;
            }
        }
        mAdvanceX = (glm::max)(mAdvanceX, (glm::max)(advanceX, int(glm::ceil(advance))));
        mAdvanceY = advanceY + mFontStyle.getLineHeight();
    }

    void addString(const glm::ivec2& position, AStringView text) noexcept override { addStringT(position, text.utf8()); }
    void addString(const glm::ivec2& position, std::u32string_view text) noexcept override { addStringT(position, text); }

    _<IRenderer::IPrerenderedString> finalize() noexcept override {
        return _new<SoftwarePrerenderedString>(std::move(mGlyphs), mAdvanceX, mAdvanceY);
    }
};


SoftwareRenderer::SoftwareRenderer() : mTexturePool([] { return nullptr; }) {}

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
        auto p1 = transform * glm::vec4(inst.position, 1.f, 1.f);
        auto p2 = transform * glm::vec4(inst.position + inst.size, 1.f, 1.f);
        for (int y = (int)p1.y; y < (int)p2.y; ++y) {
            for (int x = (int)p1.x; x < (int)p2.x; ++x) {
                putPixel({x, y}, v.color, blending);
            }
        }
    }
}
void SoftwareRenderer::gradientRectangles(const ADisplayList::GradientRectangles& v, const glm::mat4& transform, Blending blending) {}
void SoftwareRenderer::texturedRectangles(const ADisplayList::TexturedRectangles& v, const glm::mat4& transform, Blending blending) {}
void SoftwareRenderer::solidRoundedRectangles(const ADisplayList::SolidRoundedRectangles& v, const glm::mat4& transform, Blending blending) {}
void SoftwareRenderer::gradientRoundedRectangles(const ADisplayList::GradientRoundedRectangles& v, const glm::mat4& transform, Blending blending) {}
void SoftwareRenderer::texturedRoundedRectangles(const ADisplayList::TexturedRoundedRectangles& v, const glm::mat4& transform, Blending blending) {}
void SoftwareRenderer::rectangleBorders(const ADisplayList::RectangleBorders& v, const glm::mat4& transform, Blending blending) {}
void SoftwareRenderer::roundedRectangleBorders(const ADisplayList::RoundedRectangleBorders& v, const glm::mat4& transform, Blending blending) {}
void SoftwareRenderer::boxShadow(const ADisplayList::BoxShadow& v, const glm::mat4& transform, Blending blending) {}
void SoftwareRenderer::boxShadowInner(const ADisplayList::BoxShadowInner& v, const glm::mat4& transform, Blending blending) {}
void SoftwareRenderer::glyphs(const ADisplayList::Glyphs& v, const glm::mat4& transform, Blending blending) {}
void SoftwareRenderer::lines(const ADisplayList::Lines& v, const glm::mat4& transform, Blending blending) {}
void SoftwareRenderer::points(const ADisplayList::Points& v, const glm::mat4& transform, Blending blending) {}
void SoftwareRenderer::lines(const ADisplayList::LineBatches& v, const glm::mat4& transform, Blending blending) {}
void SoftwareRenderer::squareSector(const ADisplayList::SquareSector& v, const glm::mat4& transform, Blending blending) {}
void SoftwareRenderer::backdrops(const ADisplayList::Backdrop& v, const glm::mat4& transform) {}
void SoftwareRenderer::backdrops(glm::ivec2 fbSize, glm::ivec2 size, std::span<const ass::Backdrop::Preprocessed> backdrops) {}

_<IRenderer::IPrerenderedString> SoftwareRenderer::prerenderString(glm::vec2 position, const AString& text, const AFontStyle& fs) {
    if (text.empty()) return nullptr;
    SoftwareMultiStringCanvas c(fs);
    c.addString(position, text);
    return c.finalize();
}

_<IRenderer::IMultiStringCanvas> SoftwareRenderer::newMultiStringCanvas(const AFontStyle& style) {
    return _new<SoftwareMultiStringCanvas>(style);
}

glm::mat4 SoftwareRenderer::getProjectionMatrix() const { return glm::mat4(1.0f); }
_unique<ITexture> SoftwareRenderer::createNewTexture() { return nullptr; }
_unique<IRenderViewToTexture> SoftwareRenderer::newRenderViewToTexture() noexcept { return nullptr; }
