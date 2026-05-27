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

FontAtlas::FontAtlas(IRendererBackend& renderer, APixelFormat format) : mAtlas(renderer, format, { 1024, 1024 }) {}

CharacterData* FontAtlas::getCharacter(
    AChar character, const _<AFont>& font, const AFont::FontEntry& fe, ADeque<CharacterData>& cache) {
    AFont::Character& ch = font->getCharacter(fe, character);
    if (ch.empty())
        return nullptr;

    auto data = static_cast<CharacterData*>(ch.rendererData);
    if (data == nullptr) {
        auto handle = mAtlas.insert(*ch.image);
        cache.push_back(CharacterData { handle.uv, handle.texture });
        data = &cache.last();
        ch.rendererData = data;
    }
    return data;
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
    IRendererBackend& renderer, FontAtlas* fontAtlas, ADeque<CharacterData>& charDataCache, const AFontStyle& fontStyle)
  : mRenderer(renderer)
  , mFontAtlas(fontAtlas)
  , mCharDataCache(charDataCache)
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
            auto data = mFontAtlas->getCharacter(c, font, fe, mCharDataCache);
            if (!data) {
                advance += mFontStyle.getSpaceWidth();
                continue;
            }

            AFont::Character& ch = font->getCharacter(fe, c);
            int posX = (int) advance + ch.horizontal.bearing.x;
            int posY = advanceY - ch.horizontal.bearing.y;
            int width = ch.image->width();
            int height = ch.image->height();

            mGlyphs.push_back(
                { glm::vec2(posX, posY), glm::vec2(width, height), glm::vec2(data->uv.x, data->uv.y),
                  glm::vec2(data->uv.z, data->uv.w), AColor::WHITE, data->texture });

            if (hasKerning) {
                auto next = std::next(i);
                if (next != text.end()) {
                    auto kerning = font->getKerning(c, *next);
                    advance += (float) kerning.x;
                }
            }
            advance += (float) ch.horizontal.advance;
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

FontAtlas* getFontEntryData(IRendererBackend& renderer, ADeque<FontAtlas>& fontEntries, const AFontStyle& fontStyle) {
    auto fe = fontStyle.getFontEntry();
    if (fe.second.rendererData == nullptr) {
        APixelFormat format =
            (fontStyle.fontRendering == FontRendering::SUBPIXEL) ? APixelFormat::RGB_BYTE : APixelFormat::R;
        fontEntries.emplace_back(renderer, format);
        fe.second.rendererData = &fontEntries.last();
    }
    return static_cast<FontAtlas*>(fe.second.rendererData);
}

}   // namespace aui
