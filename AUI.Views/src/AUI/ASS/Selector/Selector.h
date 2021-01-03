//
// Created by alex2 on 31.12.2020.
//

#pragma once

#include <AUI/Common/AVector.h>
#include <AUI/View/AView.h>
#include <AUI/ASS/AAssHelper.h>
#include <AUI/Util/kAUI.h>
#include <utility>

class AView;


namespace ass {

    class ISubSelector {
    public:
        virtual bool isPossiblyApplicable(AView* view) = 0;
        virtual bool isStateApplicable(AView* view);
        virtual void setupConnections(AView* view, const _<AAssHelper>& helper);
    };

    class ASelector {
    private:
        AVector<_<ISubSelector>> mSubSelectors;

        template<typename SubSelector, std::enable_if_t<!std::is_pointer_v<SubSelector>, bool> = true>
        void processSubSelector(SubSelector&& subSelector) {
            ISubSelector* sub = new SubSelector(std::forward<SubSelector>(subSelector));
            mSubSelectors << std::move(_unique<ISubSelector>(sub));
        }

        template<typename SubSelector, std::enable_if_t<std::is_pointer_v<SubSelector>, bool> = true>
        void processSubSelector(SubSelector&& subSelector) {
            mSubSelectors << std::move(_unique<ISubSelector>(subSelector));
        }

        template<typename SubSelector, typename...SubSelectors>
        void processSubSelectors(SubSelector&& subSelector, SubSelectors&&... subSelectors) {
            processSubSelector(std::forward<SubSelector>(subSelector));
            processSubSelectors(std::forward<SubSelectors>(subSelectors)...);
        }

        void processSubSelectors() {}

    public:
        template<typename...SubSelectors>
        ASelector(SubSelectors&&... subSelectors) {
            processSubSelectors(std::forward<SubSelectors>(subSelectors)...);
        }
        bool isPossiblyApplicable(AView* view) const {
            for (auto& s : mSubSelectors) {
                if (s->isPossiblyApplicable(view))
                    return true;
            }
            return false;
        }
        bool isStateApplicable(AView* view) const {
            for (auto& s : mSubSelectors) {
                if (s->isStateApplicable(view))
                    return true;
            }
            return false;
        }
        void setupConnections(AView* view, const _<AAssHelper>& helper) const {
            for (auto& s : mSubSelectors) {
                if (s->isPossiblyApplicable(view)) {
                    s->setupConnections(view, helper);
                }
            }
        }

    };

    using sel = ASelector;


    // parent helpers
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
                    if (l.isStateApplicable(v) && l.isPossiblyApplicable(v)) {
                        l.setupConnections(v, helper);
                        r.setupConnections(view, helper);
                        return;
                    }
                }
            }
        }
    };

    template <typename L, typename R, std::enable_if_t<std::is_base_of_v<ISubSelector, L> && std::is_base_of_v<ISubSelector, R>, bool> = true>
    ParentSubSelector<L, R> operator>>(L&& l, R&& r) {
        return ParentSubSelector<L, R>(std::forward<L>(l), std::forward<R>(r));
    }
}