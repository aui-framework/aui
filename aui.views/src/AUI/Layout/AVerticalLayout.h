/*
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
 */

#pragma once

#include "ALayout.h"
#include <AUI/Util/LayoutDirection.h>

/**
 * @brief Places views in a column.
 * @ingroup layout_managers
 * @details
 * <dl>
 *   <dt><b>UI Builder</b></dt>
 *   <dd><code>Vertical</code></dd>
 * </dl>
 * Vertical layout manager places views in a column, fitting their width to the container's height.
 * Has a `Vertical` shortcut that allows easy usage in UI building:
 * @code{cpp}
 * setContents(
 *   Vertical {
 *     _new<AButton>("1"),
 *     _new<AButton>("2"),
 *     _new<AButton>("3"),
 *   }
 * );
 * @endcode
 *
 * <img src="https://github.com/aui-framework/aui/raw/master/docs/imgs/Screenshot_20210714_040046.png">
 * <img src="https://github.com/aui-framework/aui/raw/master/docs/imgs/vertical.jpg">
 */
class API_AUI_VIEWS AVerticalLayout : public ALayout
{
private:
	int mSpacing = 0;

public:
    // for template<> logic
    static constexpr LayoutDirection DIRECTION = LayoutDirection::VERTICAL;

	AVerticalLayout()
	{
	}
	AVerticalLayout(int spacing) : mSpacing(spacing)
	{
	}

	void onResize(int x, int y, int width, int height) override;

	int getMinimumWidth() override;
	int getMinimumHeight() override;


	void setSpacing(int spacing) override;
};