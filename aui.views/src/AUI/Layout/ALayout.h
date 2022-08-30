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
#include "AUI/Common/AObject.h"
#include "AUI/Common/ADeque.h"
#include "AUI/View/AView.h"
#include "AUI/Common/SharedPtr.h"

class AViewContainer;

/**
 * @brief Layout manager is an object that manages placement and size of @ref AView "views" inside
 * @ref AViewContainer "containers".
 * @defgroup layout_managers Layout Managers
 * @ingroup views
 * @details In AUI, layout building consists of layout managers. Layout manager determines position and size of
 * @ref AViewContainer "container"'s children @ref AView "views". A @ref AViewContainer "container" is a @ref AView
 * "view" that consists of other @ref AView "views", called children. In general, layout manager does not allow going
 * beyond the border of the @ref AViewContainer "container". A @ref AViewContainer "container" can be a child of an
 * another @ref AViewContainer "container" i.e., nesting is allowed.
 *
 * # Examples
 *
 * @ref AHorizontalLayout "Horizontal" layout:
 * <table>
 *   <tr>
 *     <th>Code</th>
 *     <th>Result</th>
 *   </tr>
 *   <tr>
 *     <td>
 *       @code{cpp}
 *       setContents(
 *         Horizontal {
 *           _new<AButton>("1"),
 *           _new<AButton>("2"),
 *           _new<AButton>("3"),
 *         }
 *       );
 *       @endcode
 *     </td>
 *     <td><img src="https://github.com/aui-framework/aui/raw/master/docs/imgs/Screenshot_20210714_035347.png" /></td>
 *   </tr>
 * </table>
 *
 * @ref AVerticalLayout "Vertical" layout:
 * <table>
 *   <tr>
 *     <th>Code</th>
 *     <th>Result</th>
 *   </tr>
 *   <tr>
 *     <td>
 *       @code{cpp}
 *       setContents(
 *         Vertical {
 *           _new<AButton>("1"),
 *           _new<AButton>("2"),
 *           _new<AButton>("3"),
 *         }
 *       );
 *       @endcode
 *     </td>
 *     <td><img src="https://github.com/aui-framework/aui/raw/master/docs/imgs/Screenshot_20210714_040046.png" /></td>
 *   </tr>
 * </table>
 *
 * Since container can be child of other container, we can create complex UIs using basic layout managers:
 * <table>
 *   <tr>
 *     <th>Code</th>
 *     <th>Result</th>
 *   </tr>
 *   <tr>
 *     <td>
 *       @code{cpp}
 *       setContents(
 *         Vertical {
 *           _new<AButton>("Up"),
 *           Horizontal {
 *               _new<AButton>("Left"),
 *               _new<AButton>("Right"),
 *           },
 *           _new<AButton>("Down"),
 *         }
 *       );
 *       @endcode
 *     </td>
 *     <td><img src="https://github.com/aui-framework/aui/raw/master/docs/imgs/Screenshot_20210714_041807.png" /></td>
 *   </tr>
 * </table>
 */

/**
 * @brief Base class for all layout managers.
 * @ingroup layout_managers
 */
class API_AUI_VIEWS ALayout: public AObject
{
protected:
	ADeque<_<AView>> mViews;
	
public:
	ALayout() = default;
	virtual ~ALayout() = default;
	virtual void onResize(int x, int y, int width, int height) = 0;
	virtual void addView(size_t index, const _<AView>& view);
	virtual void removeView(size_t index, const _<AView>& view);
	virtual int getMinimumWidth() = 0;
	virtual int getMinimumHeight() = 0;

    virtual ALayoutDirection getLayoutDirection();

    /**
     * @brief Layout spacing.
     * @param spacing spacing in px.
     */
	virtual void setSpacing(int spacing);
};
