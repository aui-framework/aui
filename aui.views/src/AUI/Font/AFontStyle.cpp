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

#include "AFontStyle.h"

#include "AUI/Platform/AFontManager.h"

size_t AFontStyle::getWidth(AStringView text) const
{
	return font->length(*this, text);
}

size_t AFontStyle::getWidth(std::u32string_view text) const
{
	return font->length(*this, text);
}

size_t AFontStyle::getLineHeight() const {
    return font->getAscenderHeight(size) * (1.f + lineSpacing);
}
