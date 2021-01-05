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
// Created by alex2772 on 1/3/21.
//

#pragma once

#include "AAssSelector.h"

namespace ass {
    template <typename L, typename R>
    struct ParentSubSelector: public IAssSubSelector {
    private:
        L l;
        R r;

    public:
        ParentSubSelector(L&& l, R&& r) : l(l), r(r) {}

        bool isPossiblyApplicable(AView* view) override {
            if (r.isPossiblyApplicable(view)) {
                for (AView* v = view->getParent(); v; v = v->getParent()) {
                    if (l.isPossiblyApplicable(v)) {
                        return true;
                    }
                }
            }
            return false;
        }

        bool isStateApplicable(AView* view) override {
            if (r.isStateApplicable(view)) {
                for (AView* v = view->getParent(); v; v = v->getParent()) {
                    if (l.isStateApplicable(v) && l.isPossiblyApplicable(v)) {
                        return true;
                    }
                }
            }
            return false;
        }

        void setupConnections(AView* view, const _<AAssHelper>& helper) override {
            if (r.isPossiblyApplicable(view)) {
                for (AView* v = view->getParent(); v; v = v->getParent()) {
                    if (l.isPossiblyApplicable(v)) {
                        l.setupConnections(v, helper);
                        r.setupConnections(view, helper);
                        return;
                    }
                }
            }
            /**
             * you should never reach here because this function is called only in case isPossiblyApplicable returned
             * true
             */
            assert(0);
        }
    };

    template <typename L, typename R, std::enable_if_t<std::is_base_of_v<IAssSubSelector, L> && std::is_base_of_v<IAssSubSelector, R>, bool> = true>
    ParentSubSelector<L, R> operator>>(L&& l, R&& r) {
        return ParentSubSelector<L, R>(std::forward<L>(l), std::forward<R>(r));
    }
}