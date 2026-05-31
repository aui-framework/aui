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

#include "FontAtlas.hpp"

#include <AUI/Render/ACanvas.hpp>
#include <AUI/Font/AFont.h>
#include <AUI/Font/AFontStyle.h>
#include <AUI/Font/FontRendering.hpp>
#include <AUI/Render/IRendererBackend.h>

namespace aui {

FontAtlas::FontAtlas(IRendererBackend& renderer, APixelFormat format) : mAtlas(renderer, format, { 1024, 1024 }, (format == APixelFormat::RGBA_BYTE) ? TextureFilter::NEAREST : TextureFilter::LINEAR) {}

CharacterData* FontAtlas::getCharacter(
    AChar character, const _<AFont>& font, const AFont::FontEntry& fe) {
    AFont::Character& ch = font->getCharacter(fe, character);
    if (ch.empty())
        return nullptr;

    GlyphKey key{ font.get(), fe.first.size, character };
    auto it = mCharData.find(key);
    if (it == mCharData.end()) {
        auto handle = mAtlas.insert(*ch.image);
        it = mCharData.emplace(key, CharacterData{ handle.uv, handle.texture }).first;
    }
    return &it->second;
}

PrerenderedString::PrerenderedString(
    AVector<GlyphInstance> glyphs, int textWidth, int textHeight, FontAtlas* fontAtlas, bool isSubpixel)
  : mGlyphs(std::move(glyphs))
  , mTextWidth(textWidth)
  , mTextHeight(textHeight)
  , mFontAtlas(fontAtlas)
  , mIsSubpixel(isSubpixel) {}

void PrerenderedString::draw(ACanvas& canvas) {
    mFontAtlas->syncWithGpu();
    for (const auto& g : mGlyphs) {
        canvas.glyphRect(g.texture, g.position, g.size, g.u1, g.u2, g.color, mIsSubpixel);
    }
}

MultiStringCanvas::MultiStringCanvas(
    IRendererBackend& renderer, FontAtlas* fontAtlas, const AFontStyle& fontStyle)
  : mRenderer(renderer)
  , mFontAtlas(fontAtlas)
  , mFontStyle(fontStyle)
  , mIsSubpixel(fontStyle.fontRendering == FontRendering::SUBPIXEL) {}

template <class UnicodeString>
void MultiStringCanvas::addStringT(const glm::ivec2& position, UnicodeString text) noexcept {
    auto& font = mFontStyle.font;
    auto fe = mFontStyle.getFontEntry();

    const bool hasKerning = font->isHasKerning();

    int advanceX = position.x;
    int advanceY = position.y;
    float advance = (float) advanceX;
    for (auto i = text.begin(); i != text.end(); ++i) {
        AChar c = *i;
        if (c == ' ') {
            advance += mFontStyle.getSpaceWidth();
        } else if (c == '\n') {
            advanceX = (glm::max) (advanceX, int(glm::ceil(advance)));
            advance = (float) position.x;
            advanceY += mFontStyle.getLineHeight();
        } else {
            auto data = mFontAtlas->getCharacter(c, font, fe);
            if (!data) {
                advance += mFontStyle.getSpaceWidth();
                continue;
            }

            AFont::Character& ch = font->getCharacter(fe, c);
            float posX = advance + ch.horizontal.bearing.x;
            float posY = advanceY - ch.horizontal.bearing.y;
            float width = ch.image->width();
            float height = ch.image->height();

            mGlyphs.push_back(
                { glm::vec2(posX, posY), glm::vec2(width, height), glm::vec2(data->uv.x, data->uv.y),
                  glm::vec2(data->uv.z, data->uv.w), mFontStyle.color, data->texture });

            if (hasKerning) {
                auto next = std::next(i);
                if (next != text.end()) {
                    auto kerning = font->getKerning(c, *next);
                    advance += (float) kerning.x;
                }
            }
            advance += ch.horizontal.advance;
        }
    }
    mAdvanceX = (glm::max) (mAdvanceX, (glm::max) (advanceX, int(glm::ceil(advance))));
    mAdvanceY = advanceY + mFontStyle.getLineHeight();
}

void MultiStringCanvas::addString(const glm::ivec2& position, AStringView text) noexcept {
    addStringT(position, text.utf8());
}
void MultiStringCanvas::addString(const glm::ivec2& position, std::u32string_view text) noexcept {
    addStringT(position, text);
}

_<IRenderer::IPrerenderedString> MultiStringCanvas::finalize() noexcept {
    return _new<PrerenderedString>(std::move(mGlyphs), mAdvanceX, mAdvanceY, mFontAtlas, mIsSubpixel);
}

AFontCache::AFontCache(IRendererBackend& renderer) : mRenderer(renderer) {}

FontAtlas& AFontCache::getGrayscaleFontAtlas() {
    if (!mGrayscaleAtlas) {
        mGrayscaleAtlas.emplace(mRenderer, APixelFormat::R);
    }
    return *mGrayscaleAtlas;
}

FontAtlas& AFontCache::getSubpixelFontAtlas() {
    if (!mSubpixelAtlas) {
        mSubpixelAtlas.emplace(mRenderer, APixelFormat::RGBA_BYTE);
    }
    return *mSubpixelAtlas;
}

FontAtlas* getFontEntryData(const AFontStyle& fontStyle, const _<AFontCache>& fontCache) {
    if (fontStyle.font && fontStyle.font->mRendererData != nullptr) {
        return static_cast<FontAtlas*>(fontStyle.font->mRendererData);
    }
    if (fontStyle.fontRendering == FontRendering::SUBPIXEL) {
        return &fontCache->getSubpixelFontAtlas();
    }
    return &fontCache->getGrayscaleFontAtlas();
}

}   // namespace aui
