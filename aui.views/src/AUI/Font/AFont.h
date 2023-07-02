// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2023 Alex2772
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

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