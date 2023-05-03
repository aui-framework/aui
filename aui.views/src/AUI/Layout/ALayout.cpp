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

#include "ALayout.h"

void ALayout::addView(size_t index, const _<AView>& view)
{
	if (index == -1) {
		mViews << view;
	} else {
		mViews.insert(mViews.begin() + index, view);
	}
}

void ALayout::removeView(size_t index, const _<AView>& view)
{

	if (index == -1) {
		mViews.removeFirst(view);
	} else {
		mViews.removeAt(index);
	}
}

void ALayout::setSpacing(int spacing)
{
}

ALayoutDirection ALayout::getLayoutDirection() {
    return ALayoutDirection::NONE;
}

const ADeque<_<AView>>& ALayout::getViews() {
    return mViews;
}
