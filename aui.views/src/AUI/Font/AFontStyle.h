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
#include "AUI/Enum/TextAlign.h"
#include "AFont.h"
#include "AUI/Render/FontRendering.h"
#include "AUI/Common/AColor.h"


class AString;


struct API_AUI_VIEWS AFontStyle {
	mutable _<AFont> font;
	unsigned size = 12;
	bool formatting = false;
	TextAlign align = TextAlign::LEFT;
	AColor color;
    bool bold = false;
    bool italic = false;

	FontRendering fontRendering = FontRendering::SUBPIXEL;
	float lineSpacing = 0.5f;

    AFontStyle();

	size_t getWidth(const AString& text) const;

	AFont::Character& getCharacter(char32_t c) {
		return font->getCharacter(getFontEntry(), c);
	}

    [[nodiscard]]
    size_t getSpaceWidth() const {
        return font->getSpaceWidth(size);
    }
	size_t getLineHeight() const;

    AFont::FontEntry getFontEntry() const {
        return font->getFontEntry({size, fontRendering});
    }

    operator AFont::FontEntry() const {
        return getFontEntry();
    }
};
