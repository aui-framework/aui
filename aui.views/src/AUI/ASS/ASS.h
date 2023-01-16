// AUI Framework - Declarative UI toolkit for modern C++17
// Copyright (C) 2020-2023 Alex2772
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

/*
 * It's not an ASS, it's Aui Style Sheets!!
 */

#pragma once

/**
 * @defgroup ass Aui Style Sheets
 * @ingroup views
 * @brief CSS-like styling system
 * @details
 * AUI uses a CSS-like stylesheet domain specific language, ASS (stands for Aui Style Sheets). Like CSS, ASS is a list
 * of rules. A rule consists of a @ref ass_selector "selector" and a list of @ref ass_declaration "declarations".
 * Selector is a basic matcher that determines whether apply a rule to the specific @ref AView "view" or not.
 * @ref ass_declaration "Declaration" controls the specific aspect of view's style (i.e. `FontSize { 18_pt }` equals
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
 *       }
 *       @endcode
 *     </td>
 *     <td>
 *       @code{cpp}
 *       {
 *         t<AButton>(),
 *         BackgroundColor { AColor::RED },
 *       },
 *       @endcode
 *     </td>
 *     <td>
 *       Set all buttons' background to red
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
 */

// Declarations
#include "Declaration/BackgroundImage.h"
#include "Declaration/BackgroundSolid.h"
#include "Declaration/BackgroundGradient.h"
#include "Declaration/BackgroundEffect.h"
#include "Declaration/Border.h"
#include "Declaration/BorderBottom.h"
#include "Declaration/BorderRadius.h"
#include "Declaration/BorderLeft.h"
#include "Declaration/BoxShadow.h"
#include "Declaration/Cursor.h"
#include "Declaration/Expanding.h"
#include "Declaration/FixedSize.h"
#include "Declaration/Font.h"
#include "Declaration/FontFamily.h"
#include "Declaration/FontRendering.h"
#include "Declaration/FontSize.h"
#include "Declaration/ImageRendering.h"
#include "Declaration/LayoutSpacing.h"
#include "Declaration/LineHeight.h"
#include "Declaration/Margin.h"
#include "Declaration/MaxSize.h"
#include "Declaration/MinSize.h"
#include "Declaration/Opacity.h"
#include "Declaration/Overflow.h"
#include "Declaration/Padding.h"
#include "Declaration/TextAlign.h"
#include "Declaration/TextBorder.h"
#include "Declaration/TextColor.h"
#include "Declaration/TextShadow.h"
#include "Declaration/TextTransform.h"
#include "Declaration/TransformOffset.h"
#include "Declaration/TransformScale.h"
#include "Declaration/VerticalAlign.h"
#include "Declaration/Visibility.h"

// Selectors
#include "AUI/ASS/Selector/AAssSelector.h"
#include "Selector/ParentSelector.h"
#include "Selector/DirectParentSelector.h"
#include "Selector/type_of.h"
#include "Selector/active.h"
#include "Selector/disabled.h"
#include "Selector/debug_selector.h"
#include "Selector/hovered.h"
#include "Selector/class_of.h"
#include "Selector/Selected.h"

// Other
#include "unset.h"
#include "AStylesheet.h"


namespace declarative {
    using namespace ass;
}