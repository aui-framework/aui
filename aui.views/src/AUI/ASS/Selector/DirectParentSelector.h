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
// Created by alex2772 on 1/3/21.
//

#pragma once

#include "AAssSelector.h"

namespace ass {
    template <typename L, typename R>
    struct DirectParentSubSelector: public IAssSubSelector {
    private:
        L l;
        R r;

    public:
        DirectParentSubSelector(L&& l, R&& r) : l(l), r(r) {}

        bool isPossiblyApplicable(AView* view) override {
            if (r.isPossiblyApplicable(view)) {
                if (auto parent = view->getParent()) {
                    return l.isPossiblyApplicable(parent);
                }
            }
            return false;
        }

        bool isStateApplicable(AView* view) override {
            if (r.isStateApplicable(view)) {
                if (auto parent = view->getParent()) {
                    return l.isStateApplicable(parent);
                }
            }
            return false;
        }

        void setupConnections(AView* view, const _<AAssHelper>& helper) override {
            auto parent = view->getParent();
            assert(parent);

            r.setupConnections(view, helper);
            l.setupConnections(parent, helper);
        }
    };

    template <typename L, typename R, std::enable_if_t<std::is_base_of_v<IAssSubSelector, L> && std::is_base_of_v<IAssSubSelector, R>, bool> = true>
    DirectParentSubSelector<L, R> operator>(L&& l, R&& r) {
        return DirectParentSubSelector<L, R>(std::forward<L>(l), std::forward<R>(r));
    }
}

#include <AUI/View/AViewContainer.h>