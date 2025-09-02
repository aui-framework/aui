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

#include <string>
#include <glm/glm.hpp>
#include <AUI/Url/AUrl.h>

#include "AUI/Render/FontRendering.h"
#include "AUI/GL/Texture2D.h"

#include "AUI/Render/SimpleTexturePacker.h"

#include "AUI/Common/AStringVector.h"
#include "AFontFamily.h"
#include <AUI/Common/AByteBuffer.h>

class AString;

class AFontManager;

class FreeType;


struct FT_FaceRec_;

class AFont {
public:
    struct Character {
        _<AImage> image;
        int advanceX, advanceY;
        int bearingX;

        [[nodiscard]]
        bool empty() const {
            return image == nullptr;
        }

        void* rendererData = nullptr;
    };

    struct FontKey {
        unsigned size;
        FontRendering fr;

        int hash() const {
            return (size << 2) | int(fr);
        }

        bool operator<(const FontKey& f) const {
            return hash() < f.hash();
        }
    };

    struct FontData {
        AVector<AOptional<Character>> characters;
        void* rendererData = nullptr;
    };


    using FontEntry = std::pair<FontKey, FontData&>;


private:
    _<FreeType> ft;
    AByteBuffer mFontDataBuffer;
    FT_FaceRec_* mFace;

    AMap<FontKey, FontData> mCharData;

    FontData& getFontEntry(unsigned size, FontRendering fr) {
        return mCharData[FontKey{size, fr}];
    }

    Character renderGlyph(const FontEntry& fs, AChar glyph);

public:
    AFont(AFontManager* fm, const AString& path);

    AFont(AFontManager* fm, const AUrl& url);

    FontEntry getFontEntry(const FontKey& key) {
        return {key, mCharData[key]};
    }

    glm::vec2 getKerning(wchar_t left, wchar_t right);

    AFont(const AFont&) = delete;

    Character& getCharacter(const FontEntry& charset, AChar glyph);

    float length(const FontEntry& charset, AStringView text);

    float length(const FontEntry& charset, std::u32string_view text);

    template<class Iterator>
    float length(const FontEntry& charset, Iterator begin, Iterator end) {
        int size = charset.first.size;
        int advance = 0;

        for (Iterator i = begin; i != end; ++i) {
            if (*i == U' ') {
                advance += getSpaceWidth(size);
            }
            else if (*i == U'\n') {
                advance = 0;
            }
            else {
                Character& ch = getCharacter(charset, *i);
                if (!ch.empty()) {
                    advance += ch.advanceX;
                    advance = glm::floor(advance);
                } else
                    advance += getSpaceWidth(size);
            }
        }
        return advance;
    }

    AString
    trimStringToWidth(const FontEntry& charset, AString::iterator begin, AString::iterator end, float maxWidth) {
        float width = 0;
        for (auto i = begin; i != end; i++) {
            if (*i == '\n') {
                return AString(begin, i);
            }
            float charWidth = length(charset, i, std::next(i));
            if (width + charWidth > maxWidth) {
                return AString(begin, i);
            }
            width += charWidth;
        }
        return AString(begin, end);
    }


    bool isHasKerning();

    [[nodiscard]]
    AString getFontFamilyName() const;

    [[nodiscard]]
    AFontFamily::Weight getFontWeight() const;

    int getAscenderHeight(unsigned size) const;

    int getDescenderHeight(unsigned size) const;

    int getSpaceWidth(unsigned size) {
        return size * 10 / 23;
    }

    [[nodiscard]]
    bool isItalic() const;
};