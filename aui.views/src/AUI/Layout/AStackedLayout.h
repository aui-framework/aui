// AUI Framework - Declarative UI toolkit for modern C++17
// Copyright (C) 2020-2022 Alex2772
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
#include "ALayout.h"


/**
 * @brief Places views in a stack (along z axis).
 * @ingroup layout_managers
 * @details
 * <dl>
 *   <dt><b>UI Builder</b></dt>
 *   <dd><code>Stacked</code></dd>
 *   <dd><code>Centered</code></dd>
 * </dl>
 * Stacked layout manager places views stacking them onto each other. If the @ref AView "view" is not expanding,
 * the @ref AView "view" is centered.
 *
 * <img src="https://github.com/aui-framework/aui/raw/master/docs/imgs/stacked2.jpg">
 */
class API_AUI_VIEWS AStackedLayout: public ALayout
{
public:
	AStackedLayout()
	{
	}
	virtual ~AStackedLayout() = default;

	void onResize(int x, int y, int width, int height) override;
	int getMinimumWidth() override;
	int getMinimumHeight() override;
};
