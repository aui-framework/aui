#pragma once
#include "Align.h"
#include "AFont.h"
#include "FontRendering.h"
#include "AUI/Common/AColor.h"


class AString;


struct FontStyle {
	_<AFont> font;
	uint8_t size = 12;
	bool formatting = false;
	Align align = ALIGN_LEFT;
	AColor color;

	FontRendering fontRendering = FR_SUBPIXEL;
	float lineSpacing = 0.5f;

	size_t getWidth(const AString& text);
};
