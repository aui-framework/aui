//
// Created by alex2 on 02.01.2021.
//

#pragma once

#include "attribute.h"

namespace ass {
    template<typename Base>
    struct active: Base, AttributeHelper<active<Base>> {

        bool isStateApplicable(AView* view) override {
            return Base::isStateApplicable(view) && view->isMousePressed();
        }

        void setupConnections(AView* view, const _<AAssHelper>& helper) override {
            Base::setupConnections(view, helper);
            view->pressedState.clearAllConnectionsWith(helper.get());
            AObject::connect(view->pressedState, slot(helper)::onInvalidateStateAss);
        }
    };
}