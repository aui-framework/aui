/**
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
 * Original code located at https://github.com/Alex2772/aui
 * =====================================================================================================================
 */

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