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

    class IAssSubSelector {
    public:
        virtual bool isPossiblyApplicable(AView* view) = 0;
        virtual bool isStateApplicable(AView* view);
        virtual void setupConnections(AView* view, const _<AAssHelper>& helper);
        virtual ~IAssSubSelector() = default;
    };

    class AAssSelector {
    private:
        AVector<_<IAssSubSelector>> mSubSelectors;

        template<typename SubSelector, std::enable_if_t<!std::is_pointer_v<SubSelector>, bool> = true>
        void processSubSelector(SubSelector&& subSelector) {
            mSubSelectors << _new<SubSelector>(std::forward<SubSelector>(subSelector));
        }

        template<typename SubSelector, std::enable_if_t<std::is_pointer_v<SubSelector>, bool> = true>
        void processSubSelector(SubSelector&& subSelector) {
            mSubSelectors << _<IAssSubSelector>(subSelector);
        }

        template<typename SubSelector, typename...SubSelectors>
        void processSubSelectors(SubSelector&& subSelector, SubSelectors&&... subSelectors) {
            processSubSelector(std::forward<SubSelector>(subSelector));
            processSubSelectors(std::forward<SubSelectors>(subSelectors)...);
        }

        void processSubSelectors() {}

    public:
        template<typename...SubSelectors>
        AAssSelector(SubSelectors&&... subSelectors) {
            processSubSelectors(std::forward<SubSelectors>(subSelectors)...);
        }
        AAssSelector(AAssSelector&& move): mSubSelectors(std::move(move.mSubSelectors)) {

        }
        AAssSelector(const AAssSelector&) = default;

        ~AAssSelector() {
            mSubSelectors.clear();
        }

        AAssSelector& operator=(const AAssSelector& c) = default;

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

    using sel = AAssSelector;
}