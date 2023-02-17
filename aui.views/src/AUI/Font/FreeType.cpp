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

#include "FreeType.h"
#include <freetype/ftlcdfil.h>
#include <freetype/ftdriver.h>
#include <freetype/ftmodapi.h>


FreeType::FreeType() {
	FT_Init_FreeType(&ft);

    FT_UInt     hinting_engine = FT_HINTING_ADOBE;



    FT_Property_Set( ft, "cff",
                     "hinting-engine", &hinting_engine );
}

FreeType::~FreeType() {
	FT_Done_FreeType(ft);
}

FT_Library FreeType::getFt() const {
	return ft;
}
