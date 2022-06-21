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
        template<typename T>
        struct Type : IAssSubSelector {
            static_assert(std::is_polymorphic_v<T>, "Your type is not polymorphic! Please define at least virtual "
                                                    "destructor for using type_of (t) style selector "
                                                    "(see https://en.cppreference.com/w/cpp/language/object#Polymorphic_objects)");
        public:
            bool isPossiblyApplicable(AView* view) override {
                return dynamic_cast<T*>(view) != nullptr;
            }

        };
    }

    template<typename T>
    struct type_of: detail::Type<T> {
    public:

        using hover = ass::hovered<detail::Type<T>>;
        using active = ass::active<detail::Type<T>>;
        using focus = ass::active<detail::Type<T>>;
        using disabled = ass::disabled<detail::Type<T>>;
    };

    template<typename T>
    using t = type_of<T>;
}