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

#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftsnames.h>
#include "AUI/Render/FreeType.h"
#include "AUI/Platform/AFontManager.h"
#include <fstream>
#include <string>
#include "AUI/Common/AStringVector.h"
#include "AFont.h"


AFont::AFont(AFontManager* fm, const AString& path) :
	ft(fm->mFreeType)
{
	if (FT_New_Face(fm->mFreeType->getFt(), path.toStdString().c_str(), 0, &mFace)) {
		throw AException("Could not load font: " + path);
	}
}

AFont::AFont(AFontManager* fm, const AUrl& url):
    ft(fm->mFreeType) {
    if (url.schema() == "file") {
        if (FT_New_Face(fm->mFreeType->getFt(), url.path().toStdString().c_str(), 0, &mFace)) {
            throw AException("Could not load font: " + url.full());
        }
        return;
    }
    mFontDataBuffer = AByteBuffer::fromStream(url.open());

    if (FT_New_Memory_Face(fm->mFreeType->getFt(), (const FT_Byte*) mFontDataBuffer.data(), mFontDataBuffer.getSize(), 0, &mFace)) {
        throw AException("Could not load font: " + url.full());
    }
}

AString AFont::getFontFamilyName() const {
    FT_SfntName name;
    FT_Get_Sfnt_Name(mFace, 0, &name);
    return std::string(name.string, name.string + name.string_len);
}

AFontFamily::Weight AFont::getFontWeight() const {
    return AFontFamily::NORMAL;
}

bool AFont::isItalic() const {
    return mFace->style_flags & FT_STYLE_FLAG_ITALIC;
}


glm::vec2 AFont::getKerning(wchar_t left, wchar_t right)
{
	FT_Vector vec2;
	FT_Get_Kerning(mFace, left, right, FT_KERNING_DEFAULT, &vec2);

	return { vec2.x >> 6, vec2.y >> 6 };
}

AFont::Character AFont::renderGlyph(const FontEntry& fs, long glyph) {
    int size = fs.first.size;
    FontRendering fr = fs.first.fr;

	FT_Set_Pixel_Sizes(mFace, 0, size);

	FT_Int32 flags = FT_LOAD_RENDER;

	if (fr == FontRendering::SUBPIXEL)
		flags |= FT_LOAD_TARGET_LCD;
	if (fr == FontRendering::NEAREST)
	    flags |= FT_LOAD_TARGET_MONO;

	FT_Error e = FT_Load_Char(mFace, glyph, flags);
	if (e) {
		throw std::runtime_error(("Cannot load char: error code" + AString::number(e)).toStdString());
	}
	FT_GlyphSlot g = mFace->glyph;
	if (g->bitmap.width && g->bitmap.rows) {
		const float div = 1.f / 64.f;
		int width = g->bitmap.width;
		
		if (fr == FontRendering::SUBPIXEL)
			width /= 3;
		
		int height = g->bitmap.rows;

		AByteBuffer data;

		if (fr == FontRendering::NEAREST) {
		    // when nearest, freetype renders glyphs into the 1bit-depth image but OpenGL required at least8bit-depth,
		    // so we will convert it here
            data.resize(g->bitmap.rows * g->bitmap.width);

            for (unsigned r = 0; r < g->bitmap.rows; ++r) {
                unsigned char* bufPtr = g->bitmap.buffer + r * g->bitmap.pitch;
                for (unsigned c = 0; c < g->bitmap.width; ++c) {
                    data.at<std::uint8_t>(c + r * g->bitmap.width) = (bufPtr[c / 8] & (0b10000000 >> (c % 8))) ? 255 : 0;
                }
            }
        } else {
            data.reserve(g->bitmap.rows * g->bitmap.pitch);

            for (unsigned r = 0; r < g->bitmap.rows; ++r) {
                unsigned char* bufPtr = g->bitmap.buffer + r * g->bitmap.pitch;
                data.write(reinterpret_cast<const char*>(bufPtr), g->bitmap.width);
            }
		}

		int imageFormat = AImageFormat::BYTE;
		if (fr == FontRendering::SUBPIXEL)
			imageFormat |= AImageFormat::RGB;
		else
			imageFormat |= AImageFormat::R;

		return Character {
            _new<AImage>(data, width, height, imageFormat),
            int(g->metrics.horiAdvance * div),
            int(-(g->metrics.horiBearingY * div) + size),
            int(g->bitmap_left)
        };
	}
    return Character{
            nullptr,
            0,
            0,
            0
    };
}

AFont::Character& AFont::getCharacter(const FontEntry& charset, long glyph) {
	auto& chars = charset.second.characters;
	if (chars.size() > glyph && chars[glyph]) {
		return *chars[glyph];
	}
	else {
		if (chars.size() <= glyph) {
            chars.resize(glyph + 1, std::nullopt);
        }
		chars[glyph] = std::move(renderGlyph(charset, glyph));

		return *chars[glyph];
	}
}

float AFont::length(const FontEntry& charset, const AString& text)
{
    int size = charset.first.size;
	int advance = 0;

	for (AString::const_iterator i = text.begin(); i != text.end(); i++) {
		if (*i == ' ')
			advance += getSpaceWidth(size);
		else if (*i == '\n')
		    advance = 0;
		else {
			Character& ch = getCharacter(charset, *i);
			if (!ch.empty()) {
                advance += ch.advanceX;
                advance = glm::floor(advance);
            }
			else
				advance += getSpaceWidth(size);
		}
	}
	return advance;
}

bool AFont::isHasKerning()
{
	return FT_HAS_KERNING(mFace);
}

int AFont::getAscenderHeight(unsigned size) const
{
	return int(mFace->ascender) * size / mFace->height;
}


int AFont::getDescenderHeight(unsigned size) const
{
	return -int(mFace->descender) * size / mFace->height;
}
