//
// Created by alex2 on 31.12.2020.
//

#pragma once

#include <AUI/Common/AVector.h>
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

        template<typename SubSelector>
        void processSubSelector(SubSelector&& subSelector) {
            ISubSelector* sub = new SubSelector(std::forward<SubSelector>(subSelector));
            mSubSelectors << std::move(_unique<ISubSelector>(sub));
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
}