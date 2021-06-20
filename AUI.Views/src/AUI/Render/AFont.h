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
 
 * Original code located at https://github.com/Alex2772/aui
 * =====================================================================================================================
 */

#pragma once

#include <string>
#include <glm/glm.hpp>
#include <AUI/Url/AUrl.h>

#include "FontRendering.h"
#include "AUI/GL/Texture2D.h"

#include "SimpleTexturePacker.h"

#include "AUI/Common/AStringVector.h"

#define FONT_SIZE 14

class AString;
class AFontManager;
class FreeType;


struct FT_FaceRec_;

class AFont {
public:
    struct Character;

private:
	_<FreeType> ft;
    AByteBuffer mFontDataBuffer;
    FT_FaceRec_* face;

	struct FontData {
		long size;
		AVector<Character*> chars;
		_<Util::SimpleTexturePacker> tp = _new<Util::SimpleTexturePacker>();
		_<GL::Texture2D> texture;
		bool isDirty = true;
		FontRendering fontRendering;
	};
	AVector<FontData> data;

	FontData& getCharsetBySize(long size, FontRendering fr);
	Character* renderGlyph(FontData& fs, long glyph, long size, FontRendering fr);
public:
	AFont(AFontManager* fm, const AString& path);
	AFont(AFontManager* fm, const AUrl& url);
	~AFont();
	glm::vec2 getKerning(wchar_t left, wchar_t right);
	AFont(const AFont&) = delete;
	Character* getCharacter(long id, long size, FontRendering fr);
	float length(const AString& text, long size, FontRendering fr);
	_<GL::Texture2D> textureOf(long size, FontRendering fr);
	_<Util::SimpleTexturePacker> texturePackerOf(long size, FontRendering fr);
	AString trimStringToWidth(const AString& text, size_t width, long size, const FontRendering& fr);
	AString trimStringToWidth(AString::const_iterator begin, AString::const_iterator end, size_t width, long size, const FontRendering& fr);
	size_t indexOfX(const AString& text, size_t x, long size, FontRendering fr);
	AStringVector trimStringToMultiline(const AString& text, int width, long size, FontRendering fr);
	bool isHasKerning();

	int getAscenderHeight(long size) const;
	int getDescenderHeight(long size) const;
};