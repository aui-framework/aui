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
 
 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
 */

//
// Created by alex2 on 03.01.2021.
//

#pragma once

#include <AUI/Util/AMetric.h>
#include "IDeclaration.h"

namespace ass {
    struct Expanding {
        unset_wrap<unsigned> expandingX;
        unset_wrap<unsigned> expandingY;

        Expanding() : expandingX(2), expandingY(2) {}
        Expanding(const unset_wrap<unsigned>& expandingX, const unset_wrap<unsigned>& expandingY) : expandingX(expandingX), expandingY(expandingY) {}
        Expanding(const unset_wrap<unsigned>& expanding) : expandingX(expanding), expandingY(expanding) {}
    };


    namespace decl {
        template<>
        struct API_AUI_VIEWS Declaration<Expanding>: IDeclarationBase {
        private:
            Expanding mInfo;

        public:
            Declaration(const Expanding& info) : mInfo(info) {
            }

            void applyFor(AView* view) override;
        };
    }
}