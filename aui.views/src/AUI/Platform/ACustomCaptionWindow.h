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
#include "ACustomWindow.h"
#include <AUI/View/AButton.h>
#include <AUI/View/ASpacerExpanding.h>
#include <AUI/Util/UIBuildingHelpers.h>
#include "CustomCaptionWindowImplWin32.h"

using CustomCaptionWindowImplCurrent = CustomCaptionWindowImplWin32;

class API_AUI_VIEWS ACustomCaptionWindow: public ACustomWindow, public CustomCaptionWindowImplCurrent
{

protected:
    bool isCustomCaptionMaximized() override;

public:

	ACustomCaptionWindow(const AString& name, int width, int height, bool stacked = false);

	ACustomCaptionWindow():
            ACustomCaptionWindow("Custom Caption Window", 240, 124)
	{
	}



	virtual ~ACustomCaptionWindow() = default;
};
