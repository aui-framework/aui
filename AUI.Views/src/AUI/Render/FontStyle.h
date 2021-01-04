#pragma once
#include "AUI/TextAlign.h"
#include "AFont.h"
#include "FontRendering.h"
#include "AUI/Common/AColor.h"


class AString;


struct FontStyle {
	mutable _<AFont> font;
	uint8_t size = 12;
	bool formatting = false;
	TextAlign align = TextAlign::LEFT;
	AColor color;

	FontRendering fontRendering = FontRendering::SUBPIXEL;
	float lineSpacing = 0.5f;

	size_t getWidth(const AString& text) const;
	size_t getLineHeight() const;
};
