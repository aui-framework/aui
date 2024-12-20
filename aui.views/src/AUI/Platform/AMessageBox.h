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
#include "AUI/Views.h"


class AString;
class AWindow;

namespace AMessageBox
{
    enum class Icon
	{
		NONE,
		INFO,
		WARNING,
		CRITICAL
	};
	enum class Button
	{
		OK,
		OK_CANCEL,
		YES_NO,
		YES_NO_CANCEL,
	};
	enum class ResultButton
	{
		INVALID,
		OK,
		CANCEL,
		YES,
		NO
	};
	API_AUI_VIEWS ResultButton show(AWindow* parent, const AString& title, const AString& message, Icon icon = Icon::NONE, Button b = Button::OK);
};
