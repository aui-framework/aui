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

#include <AUI/Render/IRenderer.h>
#include <AUI/Render/Atlas.hpp>
#include <AUI/Font/AFontStyle.h>
#include <AUI/Font/AFont.h>
#include <AUI/Render/IRendererInterfaces.h>
#include <AUI/Common/AOptional.h>

class IRendererBackend;
class ITexture;

namespace aui {

/**
 * @brief Paged texture atlas for a specific font style.
 */
class API_AUI_VIEWS FontAtlas : public aui::noncopyable {
public:
    FontAtlas(IRendererBackend& renderer, APixelFormat format);

    CharacterData* getCharacter(AChar character, const _<AFont>& font, const AFont::FontEntry& fe, ADeque<CharacterData>& cache);

    void syncWithGpu() { mAtlas.syncWithGpu(); }

private:
    Atlas mAtlas;
};

class API_AUI_VIEWS AFontCache : public aui::noncopyable {
private:
    IRendererBackend& mRenderer;
    AOptional<FontAtlas> mGrayscaleAtlas;
    AOptional<FontAtlas> mSubpixelAtlas;

public:
    AFontCache(IRendererBackend& renderer);
    ~AFontCache() = default;

    FontAtlas& getGrayscaleFontAtlas();
    FontAtlas& getSubpixelFontAtlas();
};

class API_AUI_VIEWS PrerenderedString : public IRenderer::IPrerenderedString {
private:
    AVector<GlyphInstance> mGlyphs;
    int mTextWidth;
    int mTextHeight;
    FontAtlas* mFontAtlas;
    bool mIsSubpixel;

public:
    PrerenderedString(AVector<GlyphInstance> glyphs,
                      int textWidth,
                      int textHeight,
                      FontAtlas* fontAtlas,
                      bool mIsSubpixel);

    void draw(ACanvas& canvas) override;
    int getWidth() override { return mTextWidth; }
    int getHeight() override { return mTextHeight; }
};

class API_AUI_VIEWS MultiStringCanvas : public IRenderer::IMultiStringCanvas {
private:
    AVector<GlyphInstance> mGlyphs;
    AFontStyle mFontStyle;
    FontAtlas* mFontAtlas;
    IRendererBackend& mRenderer;
    ADeque<CharacterData>& mCharDataCache;
    int mAdvanceX = 0;
    int mAdvanceY = 0;
    bool mIsSubpixel;

public:
    MultiStringCanvas(IRendererBackend& renderer,
                      FontAtlas* fontAtlas,
                      ADeque<CharacterData>& charDataCache,
                      const AFontStyle& fontStyle);

    template<class UnicodeString>
    void addStringT(const glm::ivec2& position, UnicodeString text) noexcept;

    void addString(const glm::ivec2& position, AStringView text) noexcept override;
    void addString(const glm::ivec2& position, std::u32string_view text) noexcept override;

    _<IRenderer::IPrerenderedString> finalize() noexcept override;
};

API_AUI_VIEWS FontAtlas* getFontEntryData(const AFontStyle& fontStyle,
                                const _<AFontCache>& fontCache);

}
