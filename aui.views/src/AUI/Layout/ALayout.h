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

#include <AUI/Common/AObject.h>
#include <AUI/Common/ADeque.h>
#include <AUI/Common/AFixedSizeCache.hpp>
#include <AUI/View/AView.h>
#include <AUI/Common/SharedPtr.h>
#include <AUI/Util/ALayoutDirection.h>
#include <limits>

class AViewContainer;

/**
 * @brief Layout manager is an object that manages placement and size of [views](aview.md) inside
 * [containers](aviewcontainer.md).
 * @defgroup layout-managers Layout Managers
 * @ingroup views
 * @details In AUI, layout building consists of layout managers. Layout manager determines position and size of
 * [container](aviewcontainer.md)'s children [views](aview.md). A [container](aviewcontainer.md) is a [view](aview.md)
 * that consists of other [views](aview.md), called children. In general, layout manager does not allow going
 * beyond the border of the [container](aviewcontainer.md). A [container](aviewcontainer.md) can be a child of an
 * another [container](aviewcontainer.md). Such nested combinations allows to replicate nearly any UI layout.
 *
 * This approach can be found in many popular declarative UI frameworks.
 *
 * !!! note "Design inspiration"
 *
 *     The measurement pipeline (constraint propagation, intrinsic measure, inline/block axis terminology, per-constraints
 *     caching) is heavily inspired by [Chromium's LayoutNG](https://www.chromium.org/blink/layoutng/) — the layout
 *     engine powering Chrome/Edge since ~2019. LayoutNG introduced the idea of passing an immutable
 *     `ConstraintSpace` into each layout node and getting back an immutable fragment, which maps directly to AUI's
 *     `AConstraints → measure() → layout()` pipeline.
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
 * !!! note
 *
 *     You can use [AUI Devtools](devtools.md) to play around with layouts, especially with
 *     [Expanding](#EXPANDING) property, to get better understanding on how does layout work in AUI.
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
 *     - Considering minimum sizes of child views
 *     - Adding margins and spacing
 *     - Respecting fixed size constraints
 *     - Following [aui-box-model].
 *
 * 2. **[Expanding Views](#EXPANDING)** - Children can expand to fill available space of their parent:
 *     - Set via [AView::setExpanding()] or [ass::Expanding] on a child
 *     - Requires parent to have [ass::FixedSize] or [ass::MinSize] or [EXPANDING] set to take effect
 *     - Independent for horizontal/vertical directions
 *     - Ignored if [ass::FixedSize] is set
 *
 * 3. **Spacing** - Configurable gaps between views:
 *     - Set via [ALayout::setSpacing()] or [ass::LayoutSpacing] of the parent view
 *     - Part of minimum size calculations of the parent view
 *     - Applied uniformly between its child views
 *
 * 4. **Margins** - Space around individual views:
 *     - Set per-view via ASS or margins property
 *     - Respected during layout
 *     - Part of minimum size calculations of the parent view
 *
 * 5. **Layout Direction** - Overall flow direction:
 *     - Horizontal layouts flow left-to-right
 *     - Vertical layouts flow top-to-bottom
 *     - Grid layouts use both directions
 *
 * 6. **Relativeness** - children position is relative to parent's position, not an absolute position within a window.
 *
 * 7. **Nesting** - you can nest containers into containers, and so on. When we say "container", it means a
 *    [AViewContainer]. When we say "Vertical", we imply a [AViewContainer] with [AVerticalLayout] as the
 *    layout manager.
 *
 * ---
 *
 * ## Implementing a custom AView: what to override { #CUSTOM_VIEW }
 *
 * If you are implementing a custom [AView] (say, `MyView`), here is what you need to know about the layout system.
 *
 * There are **three virtual methods** you may override in [AView]:
 *
 * | Method | Purpose | Override? |
 * |--------|---------|-----------|
 * | `onIntrinsicMeasure(AConstraints)` | Returns preferred **content** size under given constraints | **Usually** |
 * | `onComputeIntrinsicMinMaxAxis(int height)` | Returns min/max **content** width for a given height | **Sometimes** |
 * | `onLayout(glm::ivec2 size)` | Called when position/size are finalized; use to position children | Only for containers |
 *
 * !!! note "Intrinsic = content only"
 *
 *     "Intrinsic" means you deal with **content** coordinates only. The framework's public `measure()` and
 *     `computeMinMaxAxis()` wrap your intrinsic overrides and automatically account for padding, margin,
 *     `ass::FixedSize`, `ass::MinSize`, `ass::MaxSize` etc. **Never call your own intrinsic methods directly.**
 *
 * ### 1. `onIntrinsicMeasure` – the primary override
 *
 * ```cpp
 * glm::ivec2 MyView::onIntrinsicMeasure(AConstraints constraints) override {
 *     // constraints describe the *content* box available to this view.
 *     // Return the preferred content {width, height}.
 *     int w = computeMyPreferredWidth();
 *     int h = computeMyPreferredHeight();
 *     return {w, h};
 * }
 * ```
 *
 * `AConstraints` carries four fields:
 *
 * | Field | Meaning |
 * |-------|---------|
 * | `minInline` | Minimum required width (usually 0) |
 * | `maxInline` | Maximum allowed width, or `-1` if unlimited |
 * | `minBlock`  | Minimum required height (usually 0) |
 * | `maxBlock`  | Maximum allowed height, or `-1` if unlimited |
 *
 * Helper predicates: `isUnlimitedInline()`, `isUnlimitedBlock()`, `isInlineTight()`, `isBlockTight()`.
 *
 * **Real example – AAbstractLabel:**
 * ```cpp
 * glm::ivec2 AAbstractLabel::onIntrinsicMeasure(AConstraints constraints) {
 *     int measuredWidth = getFontStyle().getWidth(getTransformedText());
 *     if (mTextOverflow != ATextOverflow::NONE
 *             && !constraints.isUnlimitedInline()
 *             && measuredWidth > constraints.maxInline) {
 *         measuredWidth = constraints.maxInline;  // clamp and trigger ellipsis
 *     }
 *     return { measuredWidth, getFontStyle().getLineHeight() };
 * }
 * ```
 *
 * ### 2. `onComputeIntrinsicMinMaxAxis` – width-for-height query
 *
 * ```cpp
 * AMinMaxAxis MyView::onComputeIntrinsicMinMaxAxis(int height) override {
 *     // height is the content height, or -1 if unconstrained.
 *     // Return {min content width, max content width}.
 *     int w = computeMyContentWidth(height);
 *     return { .min = w, .max = w };
 * }
 * ```
 *
 * This is called by layout managers that need to know the range of widths a view can occupy before they have
 * committed to a height (e.g., to determine how to share horizontal space). If your view has a fixed intrinsic
 * width regardless of height, just return `{w, w}`. If the view can stretch, return different `min` and `max`.
 *
 * **When to override:** always override alongside `onIntrinsicMeasure` when your content width depends on
 * available height (e.g., word-wrapped text, aspect-ratio-locked images). For a simple fixed-content view
 * the default implementation (which delegates to `onIntrinsicMeasure`) is usually sufficient.
 *
 * ### 3. `onLayout` – finalising geometry (containers only)
 *
 * ```cpp
 * void MyContainer::onLayout(glm::ivec2 size) override {
 *     // size is the final {width, height} of this container.
 *     // Position and size each child explicitly:
 *     for (auto& child : mChildren) {
 *         glm::ivec2 childSize = child->measure(AConstraints{ .maxInline = size.x, .maxBlock = size.y });
 *         child->layout(0, currentY, childSize.x, childSize.y);
 *         currentY += childSize.y;
 *     }
 * }
 * ```
 *
 * For most leaf views (labels, images, custom-drawn widgets) you **do not** need to override `onLayout`.
 *
 * ### Summary for leaf view implementors
 *
 * If you only draw content and have no children, you need **one thing**:
 * ```cpp
 * glm::ivec2 onIntrinsicMeasure(AConstraints constraints) override;
 * ```
 * Everything else (padding, margins, fixed/min/max size, caching) is handled by the framework.
 *
 * ---
 *
 * ## Measurement pipeline in detail { #MEASURE_PIPELINE }
 *
 * The layout system has **three entry points**, from simplest to most complex:
 *
 * ```
 * onComputeIntrinsicMinMaxAxis(height)
 *        │
 *        │  (override for content min/max width)
 *        ▼
 * computeMinMaxAxis(height)    ← public; adds padding / FixedSize / MinSize / MaxSize
 *
 * onIntrinsicMeasure(constraints)
 *        │
 *        │  (override for content preferred size)
 *        ▼
 * measure(constraints)         ← public; adds padding / FixedSize / MinSize / MaxSize, cached
 *
 * onLayout(size)
 *        │
 *        │  (override in containers to position children)
 *        ▼
 * layout(x, y, w, h)           ← public; sets position & size, calls onLayout
 * ```
 *
 * **Step-by-step during a layout pass:**
 *
 * 1. Something triggers `AView::requestLayout()` (resize, style change, content change).
 * 2. On the next render frame, `AViewContainer::layout()` is invoked on the root container.
 * 3. The layout manager calls `child->computeMinMaxAxis(height)` for each child to determine
 *    how to share available space among children.
 * 4. Once widths are decided, the layout manager calls `child->measure(constraints)` to get the
 *    final preferred size for each child under the resolved constraints.
 * 5. The layout manager calls `child->layout(x, y, w, h)` which invokes `child->onLayout({w, h})`.
 * 6. Results of `measure()` are cached in `mMeasureCache` keyed by `AConstraints`; the cache is
 *    invalidated by `requestLayout()`.
 *
 * ### Axis terminology
 *
 * AUI uses CSS-inspired axis names to stay writing-mode agnostic (for future RTL/vertical text support):
 *
 * | AUI term | Meaning (LTR horizontal layout) |
 * |----------|---------------------------------|
 * | **inline** | horizontal (X) axis |
 * | **block**  | vertical (Y) axis |
 *
 * ### Relationship to Expanding / stretch factors
 *
 * `Expanding` (stretch factor) is a **secondary** mechanism layered on top of measurement. After the layout manager
 * has measured all children via `computeMinMaxAxis` / `measure`, it distributes remaining **free space** among
 * children that have a non-zero expanding value, proportionally to that value. A child with `Expanding(2)` gets twice
 * as much free space as one with `Expanding(1)`.
 *
 * Free space = container size − sum of children's measured minimum sizes.
 *
 * `ass::FixedSize` on a child makes the layout manager ignore that child's expanding value for the fixed axis.
 *
 * ---
 *
 * ## Layout Examples
 *
 * [Horizontal](ahorizontallayout.md) layout:
 * <table>
 *   <tr>
 *     <th>Code</th>
 *     <th>Result</th>
 *   </tr>
 *   <tr>
 *     <td>
 *       ```cpp
 *       setContents(
 *         Horizontal {
 *           _new<AButton>("1"),
 *           _new<AButton>("2"),
 *           _new<AButton>("3"),
 *         }
 *       );
 *       ```
 *     </td>
 *     <td><img src="https://github.com/aui-framework/aui/raw/master/docs/imgs/Screenshot_20210714_035347.png" /></td>
 *   </tr>
 * </table>
 *
 * [Vertical](averticallayout.md) layout:
 * <table>
 *   <tr>
 *     <th>Code</th>
 *     <th>Result</th>
 *   </tr>
 *   <tr>
 *     <td>
 *       ```cpp
 *       setContents(
 *         Vertical {
 *           _new<AButton>("1"),
 *           _new<AButton>("2"),
 *           _new<AButton>("3"),
 *         }
 *       );
 *       ```
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
 *       ```cpp
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
 *       ```
 *     </td>
 *     <td><img src="https://github.com/aui-framework/aui/raw/master/docs/imgs/Screenshot_20210714_041807.png" /></td>
 *   </tr>
 * </table>
 *
 * [Stacked](astackedlayout.md) layout:
 * <table>
 *   <tr>
 *     <th>Code</th>
 *     <th>Result</th>
 *   </tr>
 *   <tr>
 *     <td>
 *       ```cpp
 *       setContents(
 *         Stacked {
 *           _new<AView>() AUI_OVERRIDE_STYLE { BackgroundSolid(0xff0000_rgb), Expanding() },
 *           Label { "Test" },
 *         }
 *       );
 *       ```
 *     </td>
 *     <td><img src="https://github.com/aui-framework/aui/raw/master/docs/imgs/Screenshot_20250625_011101.png" /></td>
 *   </tr>
 * </table>
 *
 * ## Expanding { #EXPANDING }
 *
 * Expanding is a **stretch factor** — a secondary, opt-in property that tells the layout manager how to distribute
 * **free space** left over after all children have been measured.
 *
 * !!! note "Measure first, expand second"
 *
 *     The layout pipeline always runs [measure](#MEASURE_PIPELINE) first: every child reports its preferred size via
 *     `onIntrinsicMeasure`. Only *after* that does the layout manager look at Expanding to distribute whatever space
 *     remains. Expanding never replaces measurement — it only stretches views beyond their measured minimum.
 *
 * !!! note
 *
 *     You can use [AUI Devtools](devtools.md) to play around with layouts, especially with the
 *     [Expanding](#EXPANDING) property, to get better understanding of how layout works in AUI.
 *
 * Expanding is set per-axis (one value for X, one for Y), though it is convenient to set both at once.
 * Horizontal layouts ignore the Y expanding of their children; Vertical layouts ignore the X expanding.
 *
 * Views are created without any expanding by default — they take exactly as much space as their measured size
 * requires. When one or more children have a non-zero expanding value, the layout manager collects the **free space**
 * (container size minus the sum of all children's measured minimum sizes) and distributes it proportionally to the
 * expanding weights.
 *
 * Expanding a child does **not** affect the parent's own size or expanding property. To make the container itself
 * grow, use [AView::setExpanding()] on the parent, or the `Expanding` variant of declarative container notation
 * (`Vertical::Expanding`, `Horizontal::Expanding`, `Stacked::Expanding`).
 *
 * Free space of a container is its size minus the sum of its children's measured minimum sizes. Note that a
 * container normally shrinks to the minimum size required by its children. To give the container extra space to
 * distribute, set `ass::FixedSize`, `ass::MinSize`, or `Expanding` on the container itself.
 *
 * You can use ass::Expanding [ASS](ass.md) property, or AView::setExpanding method to specify Expanding:
 * <table>
 *   <tr>
 *     <th>Code</th>
 *     <th>Result</th>
 *   </tr>
 *   <tr>
 *     <td>
 *       ```cpp
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
 *         } AUI_OVERRIDE_STYLE { MinSize { 300_dp, {} } },
 *       );
 *       ```
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
 *       ```cpp
 *       setContents(
 *         Vertical {
 *           _new<AButton>("Up"),
 *           Horizontal {
 *               _new<AButton>("Left"),
 *               _new<AButton>("Center") AUI_LET { it->setExpanding(); },
 *               _new<AButton>("Right"),
 *           },
 *           _new<AButton>("Down"),
 *         } AUI_OVERRIDE_STYLE { MinSize { 300_dp, {} } },
 *       );
 *       ```
 *     </td>
 *     <td><img src="https://github.com/aui-framework/aui/raw/master/docs/imgs/Screenshot_20210714_174121.png" /></td>
 *   </tr>
 * </table>
 *
 * Expanding view does affect expanding environment inside a single container. If there's one view with expanding set to
 * any positive value it would occupy all free space in the container. If there is a view with expanding equal to 1 and
 * another view with expanding equal to 2 the first view would occupy one third of free space, the second view would
 * occupy two thirds of free space:
 * ```cpp
 * Vertical {
 *   _new<AButton>("Left") AUI_LET { it->setExpanding(1); },
 *   _new<AButton>("Right") AUI_LET { it->setExpanding(2); }, // will be twice as big as "Left"
 * }
 * ```
 *
 * You can use ASpacerExpanding as blank expanding view:
 * ```cpp
 * Vertical {
 *   _new<AButton>("Left"),
 *   SpacerExpanding(),
 *   _new<AButton>("Right"),
 * }
 * ```
 *
 * !!! note
 *
 *     FixedSize nullifies Expanding's action (on per axis basic).
 *
 * ### Applying size
 *
 * - Size of each view in tree is [calculated](#MEASURE_PIPELINE) during this phase.
 * - [AView::redraw] - triggers surface redraw. Surface applies layout before rendering.
 * - [AView::layout] - the single entry point for positioning and sizing a view. Replaces the old
 *   `applyGeometryToChildrenIfNecessary()`. Skips work if position, size and `mWantsLayoutUpdate` flag are all
 *   unchanged; otherwise sets position/size and calls [AView::onLayout(glm::ivec2)]. Always called by the parent layout
 *   manager — **do not call directly**.
 *
 * !!! note "Migration from master"
 *
 *     `applyGeometryToChildrenIfNecessary()` no longer exists. The equivalent sequence on `feat/layout-measure` is:
 *     call `requestLayout()` to invalidate caches, then let the normal render cycle drive `layout()` top-down.
 *     If you need to force an immediate synchronous layout pass (e.g. in tests), call `layout(x, y, w, h)` directly
 *     on the root container with its current geometry.
 *
 * ### Size calculation { #SIZE_CALCULATION }
 *
 * See [MEASURE_PIPELINE] for the full pipeline. In brief:
 * - [AView::computeMinMaxAxis] — returns min/max inline (width) range for a given block (height). Wraps
 *   [AView::onComputeIntrinsicMinMaxAxis] and adds padding, fixed/min/max constraints.
 * - [AView::measure] — returns preferred outer size under [AConstraints]. Wraps [AView::onIntrinsicMeasure]
 *   and adds padding, fixed/min/max constraints. Result is cached per-constraints set.
 * - After measured sizes of children are determined, the layout manager distributes remaining **free space** among
 *   children that have a non-zero [EXPANDING] value, proportionally to that value.
 *
 * ### Special cases
 *
 * - **[AScrollArea]**: requires special handling for viewport positioning and size compensation
 * - **[AForEachUI]**: manages view inflation/deflation based on visibility
 * - **Performance Optimizations**: Views outside viewport may be left unupdated to improve performance
 */

/**
 * @brief Base class for all layout managers.
 * @ingroup layout-managers
 */
class API_AUI_VIEWS ALayout : public AObject {
public:
    ALayout() = default;
    ~ALayout() override = default;

    /**
     * @brief Invalidates cached measurement results, triggering a layout recalculation.
     * @details
     * Call this whenever the layout manager's content changes in a way that may affect sizes (e.g. a child was
     * added/removed, spacing changed). Clears `mMeasureCache` and `mMinMaxSizesCache` and propagates the
     * invalidation up the view tree so that all ancestor containers re-measure themselves.
     *
     * This is the direct replacement for the old `markMinContentSizeInvalid()` from the Qt-style layout system.
     * The two are equivalent — `requestLayout()` additionally invalidates both measure caches introduced by
     * the new [MEASURE_PIPELINE].
     *
     * !!! note
     *
     *     You rarely need to call this manually. It is called automatically when ASS properties change,
     *     when a view is resized, or when [addView]/[removeView] is called. Override this in a custom layout
     *     manager only if you maintain extra internal caches that also need clearing.
     */
    virtual void requestLayout();

    /**
     * @brief Applies geometry to children.
     * @param x x coordinate in container's coordinate space, add padding if necessary.
     * @param y y coordinate in container's coordinate space, add padding if necessary.
     * @param width width of the container, add padding if necessary.
     * @param height height of the container, add padding if necessary.
     * @details
     * See [layout-managers] for more info.
     */
    virtual void layout(int x, int y, int width, int height) = 0;

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

    /**
     * @brief Returns min/max preferred inline (width) size of the layout for a given block (height).
     * @param height Content height in px, or -1 for unconstrained.
     * @return [AMinMaxAxis] with `min` and `max` outer widths.
     * @details
     * Wraps [onComputeIntrinsicMinMaxAxis] and applies padding, `ass::FixedSize`, `ass::MinSize`, `ass::MaxSize`.
     * Result is cached; invalidated by `requestLayout()`.
     * Do **not** override this method — override [onComputeIntrinsicMinMaxAxis] instead.
     * See [MEASURE_PIPELINE] for the full pipeline.
     */
    AMinMaxAxis computeMinMaxAxis(int height = -1);

    /**
     * @brief Returns the preferred outer size of the layout under given constraints.
     * @param constraints Constraints describing the available space (see [AConstraints]).
     * @return Preferred outer `{width, height}` in px.
     * @details
     * Wraps [onIntrinsicMeasure] and applies padding, `ass::FixedSize`, `ass::MinSize`, `ass::MaxSize`.
     * Result is cached per `AConstraints` key; invalidated by `requestLayout()`.
     * Do **not** override this method — override [onIntrinsicMeasure] instead.
     * See [MEASURE_PIPELINE] for the full pipeline.
     */
    glm::ivec2 measure(AConstraints constraints);

    int getMinimumWidth();
    int getMinimumHeight();
    glm::ivec2 getMinimumSize() { return { getMinimumWidth(), getMinimumHeight() }; }

    /**
     * @brief Computes the preferred **content** size under given constraints.
     * @param constraints Constraints describing the available **content** space (padding already subtracted).
     * @return Preferred content `{width, height}` in px.
     * @details
     * Override this in a layout manager to report how much space the managed children need.
     * The public [measure] wraps this method and handles padding, `ass::FixedSize`, `ass::MinSize`, `ass::MaxSize`
     * automatically — **never call** `onIntrinsicMeasure` directly.
     *
     * The default implementation delegates to [onComputeIntrinsicMinMaxAxis] and clamps to constraints.
     * See [MEASURE_PIPELINE] for the full pipeline.
     */
    virtual glm::ivec2 onIntrinsicMeasure(AConstraints constraints) {
        const auto minMax = onComputeIntrinsicMinMaxAxis(constraints.isUnlimitedBlock() ? -1 : constraints.maxBlock);
        const int maxInline = constraints.isUnlimitedInline() ? std::numeric_limits<int>::max() : constraints.maxInline;
        return {
            std::clamp(minMax.max, constraints.minInline, maxInline),
            constraints.minBlock,
        };
    }

    /**
     * @brief Computes the min/max preferred **content** inline (width) size for a given content height.
     * @param height Content height in px, or -1 for unconstrained.
     * @return [AMinMaxAxis] with `min` and `max` content widths.
     * @details
     * Override this in a layout manager to report the range of widths the managed children need.
     * The public [computeMinMaxAxis] wraps this method and handles padding, `ass::FixedSize`, etc.
     * automatically — **never call** `onComputeIntrinsicMinMaxAxis` directly.
     * See [MEASURE_PIPELINE] for the full pipeline.
     */
    virtual AMinMaxAxis onComputeIntrinsicMinMaxAxis(int height) = 0;

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

protected:
    AFixedSizeCache<AConstraints, glm::ivec2, 8> mMeasureCache;
    AFixedSizeCache<int, AMinMaxAxis, 4> mMinMaxSizesCache;
};
