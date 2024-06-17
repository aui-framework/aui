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


AUI_ENUM_FLAG(FieldCell)
{
	F_HAS_BOMB = 1,
		F_HAS_FLAG = 4,
		F_OPEN = 2,
		F_RED_BG = 8,
		F_DONT_PLANT_BOMB_HERE = 16
};
