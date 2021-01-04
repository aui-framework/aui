//
// Created by alex2772 on 1/3/21.
//

#pragma once

#include "Selector.h"

namespace ass {
    template <typename L, typename R>
    struct ParentSubSelector: public ISubSelector {
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
            if (r.isStateApplicable(view)) {
                for (AView* v = view->getParent(); v; v = v->getParent()) {
                    if (l.isPossiblyApplicable(v)) {
                        l.setupConnections(v, helper);
                        r.setupConnections(view, helper);
                        return;
                    }
                }
            }
            assert(0);
        }
    };

    template <typename L, typename R, std::enable_if_t<std::is_base_of_v<ISubSelector, L> && std::is_base_of_v<ISubSelector, R>, bool> = true>
    ParentSubSelector<L, R> operator>>(L&& l, R&& r) {
        return ParentSubSelector<L, R>(std::forward<L>(l), std::forward<R>(r));
    }
}