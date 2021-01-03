//
// Created by alex2772 on 1/3/21.
//

#pragma once

#include "attribute.h"

namespace ass {
    template<typename Base>
    struct focus: Base, AttributeHelper<focus<Base>> {
        template<typename... Args>
        focus(Args&&... args):
                Base(std::forward<Args>(args)...)
        {

        }

        bool isStateApplicable(AView* view) override {
            return Base::isStateApplicable(view) && view->hasFocus();
        }

        void setupConnections(AView* view, const _<AAssHelper>& helper) override {
            Base::setupConnections(view, helper);
            view->focusState.clearAllConnectionsWith(helper.get());
            AObject::connect(view->focusState, slot(helper)::onInvalidateStateAss);
        }
    };
}