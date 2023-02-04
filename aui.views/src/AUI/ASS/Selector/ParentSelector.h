// AUI Framework - Declarative UI toolkit for modern C++20
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

//
// Created by alex2772 on 1/3/21.
//

#pragma once

#include "AAssSelector.h"
#include <AUI/View/AViewContainer.h>

namespace ass {
    template <typename L, typename R>
    struct ParentSubSelector: public IAssSubSelector {
    private:
        L l;
        R r;

    public:
        ParentSubSelector(L l, R r) : l(std::move(l)), r(std::move(r)) {}

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