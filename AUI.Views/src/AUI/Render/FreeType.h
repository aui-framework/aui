#pragma once

#include <ft2build.h>
#include <freetype/freetype.h>
class FreeType {
private:
	FT_Library ft;
public:
	FreeType();
	~FreeType();
	FreeType(const FreeType&) = delete;
	FT_Library getFt() const;
};