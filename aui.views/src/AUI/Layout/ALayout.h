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
#include "AUI/Common/AObject.h"
#include "AUI/Common/ADeque.h"
#include "AUI/View/AView.h"
#include "AUI/Common/SharedPtr.h"
#include "AUI/Util/ALayoutDirection.h"

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
 *
 * You can specify the expanding if you want the view to grow inside it's container (AView::setExpanding):
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
 *               _new<AButton>("Center"),
 *               _new<AButton>("Right") let { it->setExpanding(); },
 *           },
 *           _new<AButton>("Down"),
 *         }
 *       );
 *       @endcode
 *     </td>
 *     <td><img src="https://github.com/aui-framework/aui/raw/master/docs/imgs/Screenshot_20210714_172900.png" /></td>
 *   </tr>
 * </table>
 *
 * Expanding views push other views in their container if necessary. This is how AExpandingSpacer works.
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
 *               _new<AButton>("Center") let { it->setExpanding(); },
 *               _new<AButton>("Right"),
 *           },
 *           _new<AButton>("Down"),
 *         }
 *       );
 *       @endcode
 *     </td>
 *     <td><img src="https://github.com/aui-framework/aui/raw/master/docs/imgs/Screenshot_20210714_174121.png" /></td>
 *   </tr>
 * </table>
 */

/**
 * @brief Base class for all layout managers.
 * @ingroup layout_managers
 */
class API_AUI_VIEWS ALayout : public AObject {
public:
    ALayout() = default;
    virtual ~ALayout() = default;
    virtual void onResize(int x, int y, int width, int height) = 0;

    /**
     * @brief Attaches view to the layout.
     * @param view view to attach.
     * @param index index to insert at. If not specified, when the view is inserted at the end.
     * @details
     * See ABasicLayout for basic implementation of this method.
     */
    virtual void addView(const _<AView>& view, AOptional<size_t> index = std::nullopt) = 0;

    /**
     * @brief Detaches view from the layout.
     * @param view view to detach.
     * @param index index of the view in the container. This value is indented for optimization purposes in most cases;
     *        the value may be ignored safely.
     *
     * @details
     * See ABasicLayout for basic implementation of this method.
     */
    virtual void removeView(aui::no_escape<AView> view, size_t index) = 0;

    virtual int getMinimumWidth() = 0;
    virtual int getMinimumHeight() = 0;

    /**
     * @brief Visits all views in the layout.
     */
    virtual AVector<_<AView>> getAllViews() = 0;

    virtual ALayoutDirection getLayoutDirection();

    /**
     * @brief Layout spacing.
     * @param spacing spacing in px.
     */
    virtual void setSpacing(int spacing);
};
