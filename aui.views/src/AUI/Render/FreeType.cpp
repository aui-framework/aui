// AUI Framework - Declarative UI toolkit for modern C++17
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
