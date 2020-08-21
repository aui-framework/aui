#include "FreeType.h"
#include <freetype/ftlcdfil.h>


FreeType::FreeType() {
	FT_Init_FreeType(&ft);

	//FT_Library_SetLcdFilter(ft, FT_LCD_FILTER_LEGACY);
	//unsigned char vals[] = { 0x02, 0x50, 0x7f, 0x50, 0x02 };
	unsigned char vals[] = { 0x03, 0x21, 0x92, 0x21, 0x03 };
	FT_Library_SetLcdFilterWeights(ft, vals);
}

FreeType::~FreeType() {
	FT_Done_FreeType(ft);
}

FT_Library FreeType::getFt() const {
	return ft;
}
