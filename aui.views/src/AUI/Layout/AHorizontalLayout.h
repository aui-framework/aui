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
