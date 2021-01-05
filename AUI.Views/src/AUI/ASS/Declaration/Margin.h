/**
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
 
 * Original code located at https://github.com/Alex2772/aui
 * =====================================================================================================================
 */

//
// Created by alex2 on 01.01.2021.
//

#pragma once

#include <AUI/Util/AMetric.h>
#include "IDeclaration.h"

namespace ass {
    struct Margin {
        ABoxFields margin;

        Margin(AMetric all) {
            margin.left = all;
            margin.top = all;
            margin.right = all;
            margin.bottom = all;
        }
        Margin(AMetric vertical, AMetric horizontal) {
            margin.left = horizontal;
            margin.top = vertical;
            margin.right = horizontal;
            margin.bottom = vertical;
        }
        Margin(AMetric top, AMetric horizontal, AMetric bottom) {
            margin.left = horizontal;
            margin.top = top;
            margin.right = horizontal;
            margin.bottom = bottom;
        }
        Margin(AMetric top, AMetric right, AMetric bottom, AMetric left) {
            margin.left = left;
            margin.top = top;
            margin.right = right;
            margin.bottom = bottom;
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