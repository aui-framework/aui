//
// Created by alex2 on 02.01.2021.
//

#pragma once

#include "attribute.h"

namespace ass {
    template<typename T>
    struct active: AttributeHelper<active<T>> {
        bool isPossiblyApplicable(AView* view) override {
            return dynamic_cast<T*>(view) != nullptr;
        }

        bool isStateApplicable(AView* view) override {
            return view->isMousePressed();
        }

        void setupConnections(AView* view, const _<AAssHelper>& helper) override {
            view->pressedState.clearAllConnectionsWith(helper.get());
            AObject::connect(view->pressedState, slot(helper)::onInvalidateStateAss);
        }
    };
}