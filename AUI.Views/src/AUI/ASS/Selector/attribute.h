//
// Created by alex2 on 02.01.2021.
//

#pragma once

#include "Selector.h"
#include <AUI/View/AView.h>

namespace ass {
    struct AttributeWrapper;

    template<typename WrappedType>
    struct AttributeHelper: virtual ISubSelector {
    public:
        AttributeWrapper operator[](const AString& attributeName);
    };

    struct AttributeWrapper: AttributeHelper<AttributeWrapper> {
    private:
        _<ISubSelector> mWrapped;
        AString mAttributeName;

    public:
        AttributeWrapper(_unique<ISubSelector>&& wrapped, const AString& attributeName):
            mWrapped(std::forward<_unique<ISubSelector>>(wrapped)),
            mAttributeName(attributeName) {}

        bool isPossiblyApplicable(AView* view) override {
            return mWrapped->isPossiblyApplicable(view);
        }

        bool isStateApplicable(AView* view) override {
            if (!mWrapped->isStateApplicable(view))
                return false;

            AMap<AString, AVariant> data;
            view->getCustomCssAttributes(data);
            if (auto c = data.contains(mAttributeName)) {
                return c->second.toBool();
            }
            return false;
        }

        void setupConnections(AView* view, const _<AAssHelper>& helper) override {
            ISubSelector::setupConnections(view, helper);
            mWrapped->setupConnections(view, helper);

            view->customCssPropertyChanged.clearAllConnectionsWith(helper.get());
            AObject::connect(view->customCssPropertyChanged, slot(helper)::onInvalidateStateAss);
        }
    };

    template<typename WrappedType>
    AttributeWrapper AttributeHelper<WrappedType>::operator[](const AString& attributeName) {
        WrappedType& t = *dynamic_cast<WrappedType*>(this);
        _unique<ISubSelector> ptr(new WrappedType(std::move(t)));
        return AttributeWrapper(std::move(ptr), attributeName);
    }
}