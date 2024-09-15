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
 * of rules. A rule consists of a @ref ass_selector "selector" and a list of @ref ass_property "properties".
 * Selector is a basic matcher that determines whether apply a rule to the specific @ref AView "view" or not. Selector
 * is the first statement in a rule and further statements are @ref ass_property "style properties".
 * @ref ass_property "Property" controls the specific aspect of view's style (i.e. `FontSize { 18_pt }` equals
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
 *     BackgroundSolid { 0xff0000_rgb },
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
 *     BackgroundSolid { 0xff0000_rgb },
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
 * ## Using with_style
 * The code below draws "Hello" label with red background and centered alignment, and "World" label with blue
 * background, using with_style:
 *
 * @code{cpp}
 * using namespace ass;
 * setContents(Centered{
 *   Label { "Hello" } with_style {
 *     BackgroundSolid { 0xff0000_rgb },
 *     TextAlign::CENTER,
 *   },
 *   Label { "World" } with_style {
 *     BackgroundSolid { 0x0000ff_rgb },
 *   },
 * })
 * @endcode
 *
 * ## Using setCustomStyle
 * In case with_style is not applicable, you may use setCustomStyle instead.
 *
 * @code{cpp}
 * using namespace ass;
 *
 * auto l = _new<ALabel>("Hello world");
 * l->setCustomStyle({
 *   BackgroundSolid { 0xff0000_rgb },
 *   TextAlign::CENTER,
 * }),
 * setContents(Centered{ l });
 * @endcode
 *
 * See below for BackgroundSolid and other declarations.
 *
 */

// Declarations
#include "Property/BackgroundImage.h"
#include "Property/BackgroundSolid.h"
#include "Property/BackgroundGradient.h"
#include "Property/BackgroundEffect.h"
#include "Property/Border.h"
#include "Property/BorderBottom.h"
#include "Property/BorderRadius.h"
#include "Property/BorderLeft.h"
#include "Property/BoxShadow.h"
#include "Property/BoxShadowInner.h"
#include "Property/Cursor.h"
#include "Property/Expanding.h"
#include "Property/FixedSize.h"
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