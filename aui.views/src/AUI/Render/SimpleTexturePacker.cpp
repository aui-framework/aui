/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2025 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "SimpleTexturePacker.h"


Util::SimpleTexturePacker::SimpleTexturePacker() {

}

Util::SimpleTexturePacker::~SimpleTexturePacker() {

}

void Util::SimpleTexturePacker::onResize(AImage& data, Util::dim side) {
	if (mImage) {
        AImage newImage(glm::uvec2(side, side), data.format());
        newImage.insert({0, 0}, *mImage);
		mImage = std::move(newImage);
	}
	else {
		mImage = AImage(glm::uvec2(side, side), data.format());
	}
}

void Util::SimpleTexturePacker::onInsert(AImage& data, const Util::dim& x, const Util::dim& y) {
    mImage->insert({x, y}, data);
}

glm::vec4 Util::SimpleTexturePacker::insert(AImage& data) {
	return TexturePacker::insert(data, (dim)data.width(), (dim)data.height());
}

