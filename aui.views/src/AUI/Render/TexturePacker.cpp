/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "TexturePacker.h"

using namespace Util;

Rect::Rect(dim x, dim y, dim width, dim height)
	: x(x),
	y(y),
	width(width),
	height(height) {
}

bool Rect::hasPoint(dim tx, dim ty) const {
	return tx >= x && tx < (x + width) && ty >= y && ty < (y + height);
}

bool Rect::collidesWith(const Rect& rect)
{
	glm::ivec2 myPos = { x, y };
	myPos *= 2;
	glm::ivec2 mySize = { width, height };
	mySize *= 2;
	myPos += mySize / 2;
	
	glm::ivec2 pos = { rect.x, rect.y };
	pos *= 2;
	glm::ivec2 size = { rect.width, rect.height };
	size *= 2;
	pos += size / 2;


	
	return !(
		glm::abs(myPos.x - pos.x) >= (mySize.x + size.x) / 2 ||
		glm::abs(myPos.y - pos.y) >= (mySize.y + size.y) / 2
		);
}


bool TexturePacker_Lol::noCollision(const Rect& r) {
	if (r.x < 0 || r.y < 0 || r.x + r.width > side || r.y + r.height > side) {
		return false;
	}
	for (Rect& t : mRects) {
		if (t.collidesWith(r)) {
			return false;
		}
	}
	return true;
}

bool TexturePacker_Lol::check(Rect& res, Rect r) {
    res = r;
    r.width += 1;
    r.height += 1;
	if (noCollision(res)) {
		mRects.push_back(r);
		return true;
	}
	return false;
}

bool TexturePacker_Lol::allocateRect(Rect& t, dim width, dim height) {
    // searching
    if (mRects.empty()) {
        Rect r(0, 0, width + 1, height + 1); // just put in (0, 0)
        mRects.push_back(r);
        t = Rect(0, 0, width, height);
        return true;
    }

	for (Rect& r : mRects) {
		if (check(t, Rect(r.x + r.width, r.y, width, height))) {
			return true;
		}
		if (check(t, Rect(r.x, r.y + r.height, width, height))) {
			return true;
		}
		/*
				if (check(t, Rect(r.x + r.width, r.y + r.height - height, width, height)))
				{
					return true;
				}

				if (check(t, Rect(r.x + r.width - width, r.y + r.height, width, height)))
				{
					return true;
				}
				*/
		if (check(t, Rect(r.x - width, r.y, width, height))) {
			return true;
		}
		if (check(t, Rect(r.x - width, r.y + r.height - height, width, height))) {
			return true;
		}

		if (check(t, Rect(r.x + r.width - width, r.y - height, width, height))) {
			return true;
		}
		if (check(t, Rect(r.x, r.y - height, width, height))) {
			return true;
		}
	}

	return false;
}
