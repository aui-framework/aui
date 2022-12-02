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
#include <AUI/Util/ALayoutDirection.h>


/**
 * @brief Places views in a row.
 * @ingroup layout_managers
 * @details
 * <dl>
 *   <dt><b>UI Builder</b></dt>
 *   <dd><code>Horizontal</code></dd>
 * </dl>
 * Horizontal layout manager places views in a row, fitting their height to the container's height.
 * Has a `Horizontal` shortcut that allows easy usage in UI building:
 * @code{cpp}
 * setContents(
 *   Horizontal {
 *     _new<AButton>("1"),
 *     _new<AButton>("2"),
 *     _new<AButton>("3"),
 *   }
 * );
 * @endcode
 *
 * <img src="https://github.com/aui-framework/aui/raw/master/docs/imgs/Screenshot_20210714_035347.png">
 * <img src="https://github.com/aui-framework/aui/raw/master/docs/imgs/horizontal.jpg">
 */
class API_AUI_VIEWS AHorizontalLayout: public ALayout
{
private:
	int mSpacing = 0;

public:
    // for template<> logic
    static constexpr ALayoutDirection DIRECTION = ALayoutDirection::HORIZONTAL;

	AHorizontalLayout()
	{
	}
	AHorizontalLayout(int spacing): mSpacing(spacing)
	{
	}

	void onResize(int x, int y, int width, int height) override;

	int getMinimumWidth() override;
	int getMinimumHeight() override;

	void setSpacing(int spacing) override;

    ALayoutDirection getLayoutDirection() override;
};
