//
// Created by alex2772 on 1/3/21.
//

#pragma once

#include "Selector.h"

namespace ass {
    template <typename L, typename R>
    struct DirectParentSubSelector: public ISubSelector {
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

    template <typename L, typename R, std::enable_if_t<std::is_base_of_v<ISubSelector, L> && std::is_base_of_v<ISubSelector, R>, bool> = true>
    DirectParentSubSelector<L, R> operator>(L&& l, R&& r) {
        return DirectParentSubSelector<L, R>(std::forward<L>(l), std::forward<R>(r));
    }
}

#include <AUI/View/AViewContainer.h>