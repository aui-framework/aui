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