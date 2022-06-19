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

//
// Created by alex2 on 01.01.2021.
//

#pragma once

#include <AUI/Util/AMetric.h>
#include "IDeclaration.h"

namespace ass {

    /**
     * @brief Controls view's @ref AView::setMargin "margins".
     * @ingroup ass
     */
    struct Margin {
        unset_wrap<AMetric> left;
        unset_wrap<AMetric> top;
        unset_wrap<AMetric> right;
        unset_wrap<AMetric> bottom;

        Margin(unset_wrap<AMetric> all):
            left(all),
            top(all),
            right(all),
            bottom(all)
        {
        }

        Margin(unset_wrap<AMetric> vertical, unset_wrap<AMetric> horizontal):
            left(horizontal),
            top(vertical),
            right(horizontal),
            bottom(vertical)
        {
        }

        Margin(unset_wrap<AMetric> top, unset_wrap<AMetric> horizontal, unset_wrap<AMetric> bottom):
            left(horizontal),
            top(top),
            right(horizontal),
            bottom(bottom)
        {
        }

        Margin(unset_wrap<AMetric> top, unset_wrap<AMetric> right, unset_wrap<AMetric> bottom, unset_wrap<AMetric> left):
            left(left),
            top(top),
            right(right),
            bottom(bottom)
        {
        }
    };

    namespace decl {
        template<>
        struct API_AUI_VIEWS Declaration<Margin>: IDeclarationBase {
        private:
            Margin mInfo;

        public:
            Declaration(const Margin& info) : mInfo(info) {

            }

            void applyFor(AView* view) override;
        };
    }
}