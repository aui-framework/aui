/**
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
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
        AVector<std::optional<Character>> characters;
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

	Character renderGlyph(const FontEntry& fs, long glyph);

public:
	AFont(AFontManager* fm, const AString& path);
	AFont(AFontManager* fm, const AUrl& url);

    FontEntry getFontEntry(const FontKey& key) {
        return { key, mCharData[key] };
    }

	glm::vec2 getKerning(wchar_t left, wchar_t right);
	AFont(const AFont&) = delete;
	Character& getCharacter(const FontEntry& charset, long glyph);
	float length(const FontEntry& charset, const AString& text);
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