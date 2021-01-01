//
// Created by alex2 on 31.12.2020.
//

#pragma once

#include <AUI/Common/AVector.h>
#include <utility>

class AView;

namespace ass {
    class ISubSelector {
    public:
        virtual bool isApplicable(AView* view) = 0;
    };

    template<typename T>
    class any: public ISubSelector {
    public:
        bool isApplicable(AView* view) override {
            return dynamic_cast<T*>(view) != nullptr;
        }

        struct hover: public ISubSelector {
            bool isApplicable(AView* view) override {
                return dynamic_cast<T*>(view) != nullptr;
            }
        };
    };

    template<>
    class any<AView>: public ISubSelector {
    public:
        bool isApplicable(AView* view) override {
            return true;
        }
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
        bool isApplicable(AView* view) const {
            for (auto& s : mSubSelectors) {
                if (s->isApplicable(view))
                    return true;
            }
            return false;
        }
    };

    using sel = ASelector;
}