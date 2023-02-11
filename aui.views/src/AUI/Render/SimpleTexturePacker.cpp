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

#include "SimpleTexturePacker.h"


Util::SimpleTexturePacker::SimpleTexturePacker() {

}

Util::SimpleTexturePacker::~SimpleTexturePacker() {

}

void Util::SimpleTexturePacker::onResize(AImage& data, Util::dim side) {
	if (mImage) {
		mImage = AImage::resize(*mImage, static_cast<uint16_t>(side), static_cast<uint16_t>(side));
	}
	else {
		mImage = AImage(side, side, data.getFormat());
	}
}

void Util::SimpleTexturePacker::onInsert(AImage& data, const Util::dim& x, const Util::dim& y) {
	AImage::copy(data, *mImage, x, y);
}

glm::vec4 Util::SimpleTexturePacker::insert(AImage& data) {
	return TexturePacker::insert(data, (dim)data.getWidth(), (dim)data.getHeight());
}

