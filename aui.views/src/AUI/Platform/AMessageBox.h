﻿// AUI Framework - Declarative UI toolkit for modern C++20
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

#pragma once
#include "AUI/Util/EnumUtil.h"
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
