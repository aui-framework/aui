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

#pragma once
#include "AUI/Reflect/AEnumerate.h"


/**
 * @brief Controls the expanding of AView.
 * @ingroup ass
 * @ingroup views
 */
enum class FontRendering
{
	NEAREST = 0,
	ANTIALIASING = 1,
	SUBPIXEL = 2,
};

AUI_ENUM_VALUES(FontRendering, FontRendering::NEAREST, FontRendering::ANTIALIASING, FontRendering::SUBPIXEL)
