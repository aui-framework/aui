#pragma once

#include <string>
#include <glm/glm.hpp>
#include <AUI/Url/AUrl.h>

#include "FontRendering.h"
#include "AUI/GL/Texture.h"

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
		_<GL::Texture> texture;
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
	_<GL::Texture> textureOf(long size, FontRendering fr);
	_<Util::SimpleTexturePacker> texturePackerOf(long size, FontRendering fr);
	AString trimStringToWidth(const AString& text, size_t width, long size, FontRendering fr);
	size_t indexOfX(const AString& text, size_t x, long size, FontRendering fr);
	AStringVector trimStringToMultiline(const AString& text, int width, long size, FontRendering fr);
	bool isHasKerning();

	int getAscenderHeight(long size) const;
	int getDescenderHeight(long size) const;
};