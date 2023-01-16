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
        DirectParentSubSelector(L l, R r) : l(std::move(l)), r(std::move(r)) {}

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