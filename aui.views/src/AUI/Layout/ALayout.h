/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2025 Alex2772 and Contributors
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
 * The ALayout is the base class for all layout managers in AUI. Layout managers are responsible for:
 *
 * - Positioning child views within their container
 * - Calculating minimum sizes
 * - Handling view additions and removals
 * - Managing spacing between views
 * - Respecting view margins and alignment
 * - Supporting expanding/stretching of views
 *
 * @note
 * You can use @ref docs/Devtools.md "AUI Devtools" to play around with layouts, especially with
 * @ref EXPANDING "Expanding" property, to get better understanding on how does layout work in AUI.
 *
 * Common layout managers include:
 *
 * - **AHorizontalLayout** - Arranges views in a horizontal row
 * - **AVerticalLayout** - Arranges views in a vertical column
 * - **AStackedLayout** - Centers views, displaying them on top of each other
 * - **AAdvancedGridLayout** - Arranges views in a grid with customizable cell sizing
 *
 * Key concepts:
 *
 * 1. **Minimum Size** - Layout managers calculate minimum size requirements by:
 *    - Considering minimum sizes of child views
 *    - Adding margins and spacing
 *    - Respecting fixed size constraints
 *    - Following @ref "AUI Box Model".
 *
 * 2. **@ref EXPANDING "Expanding Views"** - Children can expand to fill available space of their parent:
 *    - Set via @ref AView::setExpanding or @ref ass::Expanding on a child
 *    - Requires parent to have @ref ass::FixedSize or @ref ass::MinSize or @ref EXPANDING set to take effect
 *    - Independent for horizontal/vertical directions
 *    - Ignored if @ref ass::FixedSize is set
 *
 * 3. **Spacing** - Configurable gaps between views:
 *    - Set via @ref ALayout::setSpacing() or @ref ass::LayoutSpacing of the parent view
 *    - Part of minimum size calculations of the parent view
 *    - Applied uniformly between its child views
 *
 * 4. **Margins** - Space around individual views:
 *    - Set per-view via ASS or margins property
 *    - Respected during layout
 *    - Part of minimum size calculations of the parent view
 *
 * 5. **Layout Direction** - Overall flow direction:
 *    - Horizontal layouts flow left-to-right
 *    - Vertical layouts flow top-to-bottom
 *    - Grid layouts use both directions
 *
 * 6. **Relativeness** - children position is relative to parent's position, not an absolute position within a window.
 *
 * 7. **Nesting** - you can nest containers into containers, and so on. When we say "container", it means a
 *    @ref AViewContainer. When we say "Vertical", we imply a @ref AViewContainer with @ref AVerticalLayout as the
 *    layout manager.
 *
 * # Layout Examples
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
 * @ref AStackedLayout "Stacked" layout:
 * <table>
 *   <tr>
 *     <th>Code</th>
 *     <th>Result</th>
 *   </tr>
 *   <tr>
 *     <td>
 *       @code{cpp}
 *       setContents(
 *         Stacked {
 *           _new<AView>() with_style { BackgroundSolid(0xff0000_rgb), Expanding() },
 *           Label { "Test" },
 *         }
 *       );
 *       @endcode
 *     </td>
 *     <td>
 *     @image html docs/imgs/Screenshot_20250625_011101.png
 *     </td>
 *   </tr>
 * </table>
 *
 * # Expanding {#EXPANDING}
 * Expanding (often referred as stretch factor) is a property of any AView. Expanding is an expansion coefficient set on
 * per-axis basic (i.e, one value along x axis, another value along y axis), however it's convenient to set both values.
 * Hints layout manager how much this AView should be extended relative to other AViews in the same container.
 *
 * @note
 * You can use @ref docs/Devtools.md "AUI Devtools" to play around with layouts, especially with
 * @ref EXPANDING "Expanding" property, to get better understanding on how does layout work in AUI.
 *
 * Horizontal layouts ignore y expanding of their children, Vertical layouts ignore x expanding of their children.
 *
 * Views are normally created without any expanding set. When Expanding views appear in a layout they are given a share
 * of space in accordance with their expanding or their minimum size whichever gives more space to them. Expanding is
 * used to change how much space views are given in proportion to one another.
 *
 * Expanding view does not affect parent's size or parent's expanding property. Use AView::setExpanding() on parent, or
 * `Expanding` variant of declarative container notation (`Vertical::Expanding`, `Horizontal::Expanding`,
 * `Stacked::Expanding`) for such case.
 *
 * Expanding views use free space of their container to grow.
 *
 * Free space of a container is determined by its size subtracted by sum of minimum sizes of its children. Please note
 * that your container would probably occupy minimum possible size (determined by minimum sizes of its children). It
 * order to make container larger than minimum possible size, you can specify FixedSize or MinSize or Expanding to the
 * container.
 *
 * You can use ass::Expanding @ref ass "ASS" property, or AView::setExpanding method to specify Expanding:
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
 *             _new<AButton>("Left"),
 *             _new<AButton>("Center"),
 *             _new<AButton>("Right") AUI_LET { it->setExpanding(); },
 *                  // alias to it->setExpanding(2) ^^^^^^
 *           },
 *           _new<AButton>("Down"),
 *         } with_style { MinSize { 300_dp, {} } },
 *       );
 *       @endcode
 *     </td>
 *     <td><img src="https://github.com/aui-framework/aui/raw/master/docs/imgs/Screenshot_20210714_172900.png" /></td>
 *   </tr>
 * </table>
 *
 * Expanding views push remaining views in their container:
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
 *               _new<AButton>("Center") AUI_LET { it->setExpanding(); },
 *               _new<AButton>("Right"),
 *           },
 *           _new<AButton>("Down"),
 *         } with_style { MinSize { 300_dp, {} } },
 *       );
 *       @endcode
 *     </td>
 *     <td><img src="https://github.com/aui-framework/aui/raw/master/docs/imgs/Screenshot_20210714_174121.png" /></td>
 *   </tr>
 * </table>
 *
 * Expanding view does affect expanding environment inside a single container. If there's one view with expanding set to
 * any positive value it would occupy all free space in the container. If there is a view with expanding equal to 1 and
 * another view with expanding equal to 2 the first view would occupy one third of free space, the second view would
 * occupy two thirds of free space:
 * @code{cpp}
 * Vertical {
 *   _new<AButton>("Left") AUI_LET { it->setExpanding(1); },
 *   _new<AButton>("Right") AUI_LET { it->setExpanding(2); }, // will be twice as big as "Left"
 * }
 * @endcode
 *
 * You can use ASpacerExpanding as blank expanding view:
 * @code{cpp}
 * Vertical {
 *   _new<AButton>("Left"),
 *   SpacerExpanding(),
 *   _new<AButton>("Right"),
 * }
 * @endcode
 *
 * @note
 * FixedSize nullifies Expanding's action (on per axis basic).
 *
 * # Implementation details
 *
 * The process of applying position and size involves several key functions:
 * ```
 * AWindow::redraw()
 * └─> AWindow::applyGeometryToChildrenIfNecessary()
 *     └─> AWindow::applyGeometryToChildren()
 *         └─> ALayout::onResize()                                                  ┐
 *             └─> AViewContainerBase::getMinimumSize()              ┐              │
 *                 └─> AViewContainerBase::getContentMinimumWidth()  │              │
 *                     └─> ALayout::getMinimumWidth()                │              │
 *                         └─> AView::getMinimumWidth()              │ cached       │
 *                 └─> AViewContainerBase::getContentMinimumHeight() │              │ potentially
 *                     └─> ALayout::getMinimumHeight()               │              │ recursive
 *                         └─> AView::getMinimumHeight()             ┘              │
 *             └─> AViewContainerBase::setGeometry()                                │
 *                 └─> AViewContainerBase::setSize()                                │
 *                     └─> AViewContainerBase::applyGeometryToChildrenIfNecessary() │
 *                         └─> AViewContainerBase::applyGeometryToChildren()        │
 *                             └─> ALayout::onResize()                              ┘
 *                                 └─> AView::setGeometry()
 * ```
 *
 * ## Applying size
 *
 * - Size of each view in tree is @ref SIZE_CALCULATION "calculated" on this phase
 * - @ref AView::redraw "AWindow::redraw" - geometry is applied before rendering
 * - @ref AViewContainerBase::applyGeometryToChildrenIfNecessary "applyGeometryToChildrenIfNecessary" - applies geometry
 *   only if really needed (i.e., if there were a resize event, or views were added or removed)
 * - @ref AViewContainerBase::applyGeometryToChildren "applyGeometryToChildren" - applies geometry to its children with
 *   no preconditions
 * - @ref ALayout::onResize - implemented by layout manager, whose have their own algorithms of arranging views
 * - @ref AView::setGeometry "setGeometry" - sets geometry of a view (which might be a container)
 *
 * ## Size calculation {#SIZE_CALCULATION}
 *
 * - Layout manager queries **Minimum size** which is determined with @ref AView::getMinimumSize and cached until the
 *   view or its children call @ref AView::markMinContentSizeInvalid. It considers:
 *     - Children's minimum sizes (if any). A child includes its @ref ass::Padding to its minimum size.
 *     - Children's @ref ass::Margin
 *     - Container's @ref ass::Padding
 *     - Container's @ref ass::LayoutSpacing
 *     - Other constraints such as @ref ass::FixedSize
 * - After minimum sizes of children are calculated, layout manager queries their **expanding** ratios, and gives such
 *   views a share of free space if available. Unlike minimum size, @ref EXPANDING ratio does not depend on children's
 *   @ref EXPANDING ratios.
 *
 * ## Special cases
 *
 * - **@ref AScrollArea**: requires special handling for viewport positioning and size compensation
 * - **@ref AForEachUI**: manages view inflation/deflation based on visibility
 * - **Performance Optimizations**: Views outside viewport may be left unupdated to improve performance
 */

/**
 * @brief Base class for all layout managers.
 * @ingroup layout_managers
 */
class API_AUI_VIEWS ALayout : public AObject {
public:
    ALayout() = default;
    virtual ~ALayout() = default;

    /**
     * @brief Applies geometry to children.
     * @param x x coordinate in container's coordinate space, add padding if necessary.
     * @param y y coordinate in container's coordinate space, add padding if necessary.
     * @param width width of the container, add padding if necessary.
     * @param height height of the container, add padding if necessary.
     * @details
     * See @ref layout_managers for more info.
     */
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
