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
// Created by alex2 on 02.01.2021.
//

#pragma once

#include <AUI/Util/kAUI.h>
#include <AUI/View/AView.h>
#include <AUI/ASS/AAssHelper.h>

#include "hovered.h"
#include "active.h"
#include "focus.h"
#include "disabled.h"

namespace ass {

    namespace detail {
        struct ClassOf: IAssSubSelector {
        private:
            AStringVector mClasses;
            
        public:
            ClassOf(const AStringVector& classes) : mClasses(classes) {}
            ClassOf(const AString& clazz) : mClasses({clazz}) {}

            bool isPossiblyApplicable(AView* view) override {
                for (auto& v : mClasses) {
                    if (view->getCssNames().contains(v)) {
                        return true;
                    }
                }
                return false;
            }

            bool isStateApplicable(AView* view) override {
                return isPossiblyApplicable(view);
            }

            const AStringVector& getClasses() const {
                return mClasses;
            }
        };
    }

    struct class_of: detail::ClassOf {
    public:
        class_of(const AStringVector& classes) : ClassOf(classes) {}
        class_of(const AString& clazz) : ClassOf(clazz) {}

        using hover = ass::hovered<detail::ClassOf>;
        using active = ass::active<detail::ClassOf>;
        using focus = ass::focus<detail::ClassOf>;
        using disabled = ass::disabled<detail::ClassOf>;
    };

    using c = class_of;
}