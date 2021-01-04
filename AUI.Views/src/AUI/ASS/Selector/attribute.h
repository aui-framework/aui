//
// Created by alex2 on 02.01.2021.
//

#pragma once

#include "AAssSelector.h"
#include <AUI/View/AView.h>

#pragma warning(disable: 4250)

namespace ass {
    struct AttributeWrapper;

    template<typename WrappedType>
    struct AttributeHelper: virtual IAssSubSelector {
    public:
        AttributeWrapper operator[](const AString& attributeName);
    };

    struct AttributeWrapper: AttributeHelper<AttributeWrapper> {
    private:
        _<IAssSubSelector> mWrapped;
        AString mAttributeName;

    public:
        AttributeWrapper(_unique<IAssSubSelector>&& wrapped, const AString& attributeName):
            mWrapped(std::forward<_unique<IAssSubSelector>>(wrapped)),
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
            IAssSubSelector::setupConnections(view, helper);
            mWrapped->setupConnections(view, helper);

            view->customCssPropertyChanged.clearAllConnectionsWith(helper.get());
            AObject::connect(view->customCssPropertyChanged, slot(helper)::onInvalidateStateAss);
        }
    };

    template<typename WrappedType>
    AttributeWrapper AttributeHelper<WrappedType>::operator[](const AString& attributeName) {
        WrappedType& t = *dynamic_cast<WrappedType*>(this);
        _unique<IAssSubSelector> ptr(new WrappedType(std::move(t)));
        return AttributeWrapper(std::move(ptr), attributeName);
    }
}