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

/*
 * It's not an ASS, it's Aui Style Sheets!!
 */

#pragma once

/**
 * @defgroup ass Aui Style Sheets
 * @ingroup views
 * @brief CSS-like styling system
 * @details
 * AUI uses CSS-like stylesheet domain specific language, ASS (stands for Aui Style Sheets). Like CSS, ASS is a list
 * of rules. A rule consists of a [ass_selectors] and a list of [ass_properties] . Selector is a basic matcher
 * that determines whether apply a rule to the specific [view](AView) or not. Selector is the first statement in a
 * rule and further statements are [style properties](ass_properties).
 * [Property](ass_properties) controls the specific aspect of view's style (i.e. `FontSize { 18_pt }` equals
 * "call setFontSize(18_pt) for every matched view").
 * <table>
 *   <tr>
 *     <th>CSS</th>
 *     <th>AUI ASS</th>
 *     <th>Meaning</th>
 *   </tr>
 *   <tr>
 *     <td>
 *       @code{css}
 *       AButton {
 *         background: red;
 *         color: white;
 *       }
 *       @endcode
 *     </td>
 *     <td>
 *       @code{cpp}
 *       {
 *         t<AButton>(),
 *         BackgroundColor { AColor::RED },
 *         TextColor { AColor::WHITE },
 *       },
 *       @endcode
 *     </td>
 *     <td>
 *       Set all buttons' background to red and text color to white
 *     </td>
 *   </tr>
 *   <tr>
 *     <td>
 *       @code{css}
 *       ATextField::hover {
 *         border: 1px solid #00f;
 *       }
 *       @endcode
 *     </td>
 *     <td>
 *       @code{cpp}
 *       {
 *         t<ATextField>::hover(),
 *         Border { 1_dp, 0x0000ff_rgb },
 *       },
 *       @endcode
 *     </td>
 *     <td>
 *       Set all text fields' border to #00f (blue) on hover
 *     </td>
 *   </tr>
 * </table>
 *
 * ## Using global style
 * Global stylesheet is applied to the whole program.
 *
 * Somewhere in your entry point, you may write:
 * @code{cpp}
 * AStylesheet::global().addRules({
 *   {
 *     t<ALabel>(),
 *     BackgroundSolid { AColor::RED },
 *     TextAlign::CENTER,
 *   },
 *   {
 *     t<AButton>(),
 *     BackgroundSolid { 0x000000_rgb },
 *     TextAlign::CENTER,
 *   },
 * });
 * @endcode
 *
 * ## Using container stylesheet
 * Container stylesheet is applied only to children (both direct and indirect) of the container.
 *
 * @code{cpp}
 * container->setExtraStylesheet({
 *   {
 *     t<ALabel>(),
 *     BackgroundSolid { AColor::RED },
 *     TextAlign::CENTER,
 *   },
 *   {
 *     t<AButton>(),
 *     BackgroundSolid { 0x000000_rgb },
 *     TextAlign::CENTER,
 *   },
 * });
 * @endcode
 *
 * ## Using AUI_WITH_STYLE
 * The code below draws "Hello" label with red background and centered alignment, and "World" label with blue
 * background, using AUI_WITH_STYLE:
 *
 * @code{cpp}
 * using namespace ass;
 * setContents(Centered{
 *   Label { "Hello" } AUI_WITH_STYLE {
 *     BackgroundSolid { AColor::RED },
 *     TextAlign::CENTER,
 *   },
 *   Label { "World" } AUI_WITH_STYLE {
 *     BackgroundSolid { 0x0000ff_rgb },
 *   },
 * })
 * @endcode
 *
 * ## Using setCustomStyle
 * In case AUI_WITH_STYLE is not applicable, you may use setCustomStyle instead.
 *
 * @code{cpp}
 * using namespace ass;
 *
 * auto l = _new<ALabel>("Hello world");
 * l->setCustomStyle({
 *   BackgroundSolid { AColor::RED },
 *   TextAlign::CENTER,
 * }),
 * setContents(Centered{ l });
 * @endcode
 *
 * # Selectors
 * As said earlier, first statement in a rule is selector. Here's some examples.
 *
 * ## Select all ALabels
 * @code{cpp}
 * using namespace ass;
 * AStylesheet::global().addRules({
 *   {
 *     t<ALabel>(),
 *     BackgroundSolid { AColor::RED },
 *     TextAlign::CENTER,
 *   },
 * });
 * @endcode
 *
 * ## Select all ASS name
 * @code{cpp}
 * using namespace ass;
 * AStylesheet::global().addRules({
 *   {
 *     c(".highlight"),
 *     BackgroundSolid { AColor::RED },
 *     TextAlign::CENTER,
 *   },
 * });
 * // ...
 * auto v = _new<AView>();
 * v->addAssName(".highlight");
 * setContents(Centered { v });
 * // or
 * setContents(Centered {
 *     _new<AView>() << ".highlight",
 * });
 * @endcode
 *
 * ## Select all ALabel's or AButton's
 * @code{cpp}
 * using namespace ass;
 * AStylesheet::global().addRules({
 *   {
 *     {t<ALabel>(), t<AButton>()},
 *     BackgroundSolid { AColor::RED },
 *     TextAlign::CENTER,
 *   },
 * });
 * @endcode
 *
 * ## Select all labels with ASS name
 * @code{cpp}
 * using namespace ass;
 * AStylesheet::global().addRules({
 *   {
 *     t<ALabel>() && c(".highlight"),
 *     BackgroundSolid { AColor::RED },
 *   },
 * });
 * // ...
 * setContents(Centered {
 *     Label { "Highlighted" } << ".highlight",
 *     Label { "Not highlighted" },
 *     Button { "Not highlighted either" },
 * });
 * @endcode
 *
 * ## Select indirect child
 * @code{cpp}
 * using namespace ass;
 * AStylesheet::global().addRules({
 *   {
 *     c(".highlight_container") >> t<ALabel>(),
 *     BackgroundSolid { AColor::RED },
 *     TextAlign::CENTER,
 *   },
 * });
 * // ...
 * setContents(Centered {
 *     Vertical {
 *         Label { "Highlighted" },
 *         Centered { Label { "Highlighted" } },
 *     } << ".highlight_container",
 *     Vertical {
 *         Label { "Not highlighted" },
 *     },
 * });
 * @endcode
 *
 * ## Select direct child
 * Works faster than selecting indirect child
 * @code{cpp}
 * using namespace ass;
 * AStylesheet::global().addRules({
 *   {
 *     c(".highlight_container") > t<ALabel>(),
 *     BackgroundSolid { AColor::RED },
 *     TextAlign::CENTER,
 *   },
 * });
 * // ...
 * setContents(Centered {
 *     Vertical {
 *         Label { "Highlighted" },
 *         Centered { Label { "Not highlighted" } },
 *     } << ".highlight_container",
 *     Vertical {
 *         Label { "Not highlighted" },
 *     },
 * } << ".highlight_container");
 * @endcode
 *
 * ## Sub selectors
 * Sub selector is kind of a selector that depends on view's state (i.e, pressed or focused). Sub selectors, as well as
 * other selectors don't replace previous rules entirely. Instead, they extend existing rules. However, same properties
 * are replaced.
 *
 * ## Hover sub selector
 * Hovered view is a view below mouse cursor.
 * @code{cpp}
 * using namespace ass;
 * AStylesheet::global().addRules({
 *   {
 *     t<ALabel>::hover(),
 *     BackgroundSolid { AColor::RED },
 *     TextAlign::CENTER,
 *   },
 * });
 * @endcode
 *
 * ## Active sub selector
 * Active view is a pressed view.
 * @code{cpp}
 * using namespace ass;
 * AStylesheet::global().addRules({
 *   {
 *     t<ALabel>::hover(),
 *     BackgroundSolid { AColor::RED },
 *   },
 * });
 * @endcode
 *
 * ## Focus sub selector
 * Focused view is a view that was clicked and currently receiving keyboard input.
 * @code{cpp}
 * using namespace ass;
 * AStylesheet::global().addRules({
 *   {
 *     t<ALabel>::focus(),
 *     Border { 1_px, AColor::RED },
 *     TextColor { AColor::RED },
 *   },
 * });
 * @endcode
 *
 * ## Disabled sub selector
 * Disabled view is a view with AView::setEnabled(false) thus not able to change it's state.
 * @code{cpp}
 * using namespace ass;
 * AStylesheet::global().addRules({
 *   {
 *     t<ALabel>::disabled(),
 *     Border { 1_px, AColor::RED },
 *     TextColor { AColor::RED },
 *   },
 * });
 * @endcode
 *
 * # Box Model (Padding, Margin)
 *
 * See ["AUI] Box Model".
 *
 * # ASS Refenence
 * See below for declarations and selectors.
 *
 */

/**
 * @defgroup ass_selectors ASS selectors
 * @ingroup ass
 * @details
 * AUI Style Sheets selector is a predicate whether view suit certain rules or not.
 */

/**
* @defgroup ass_properties ASS properties
* @ingroup ass
* @details
* AUI Style Sheets property customizes view's appearance and behavior.
*/

// Declarations
#include "Property/Backdrop.h"
#include "Property/BackgroundImage.h"
#include "Property/BackgroundSolid.h"
#include "Property/BackgroundGradient.h"
#include "Property/BackgroundEffect.h"
#include "Property/Border.h"
#include "Property/BorderBottom.h"
#include "Property/BorderLeft.h"
#include "Property/BorderRadius.h"
#include "Property/BorderRight.h"
#include "Property/BorderTop.h"
#include "Property/BoxShadow.h"
#include "Property/BoxShadowInner.h"
#include "Property/Cursor.h"
#include "Property/CustomProperty.h"
#include "Property/Expanding.h"
#include "Property/FixedSize.h"
#include "Property/Float.h"
#include "Property/Font.h"
#include "Property/FontFamily.h"
#include "Property/FontRendering.h"
#include "Property/FontSize.h"
#include "Property/ImageRendering.h"
#include "Property/LayoutSpacing.h"
#include "Property/LineHeight.h"
#include "Property/Margin.h"
#include "Property/MaxSize.h"
#include "Property/MinSize.h"
#include "Property/Opacity.h"
#include "Property/Overflow.h"
#include "Property/Padding.h"
#include "Property/TextAlign.h"
#include "Property/TextBorder.h"
#include "Property/TextColor.h"
#include "Property/TextShadow.h"
#include "Property/TextTransform.h"
#include "Property/TransformOffset.h"
#include "Property/TransformRotate.h"
#include "Property/TransformScale.h"
#include "Property/VerticalAlign.h"
#include "Property/Visibility.h"
#include "Property/ATextOverflow.h"
#include "Property/ScrollbarAppearance.h"

#include <AUI/ASS/AAssHelper.h>

// Selectors
#include "AUI/ASS/Selector/AAssSelector.h"
#include "Selector/ParentSelector.h"
#include "Selector/DirectParentSelector.h"
#include "Selector/AndSelector.h"
#include "Selector/NotSelector.h"
#include "Selector/type_of.h"
#include "Selector/activated.h"
#include "Selector/disabled.h"
#include "Selector/debug_selector.h"
#include "Selector/hovered.h"
#include "Selector/class_of.h"
#include "Selector/Selected.h"
#include "Selector/on_state.h"

// Other
#include "unset.h"
#include "AStylesheet.h"
#include "PropertyListRecursive.h"


namespace declarative {
    using namespace ass;
}