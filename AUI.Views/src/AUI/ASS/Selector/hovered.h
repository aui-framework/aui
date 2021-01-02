//
// Created by alex2 on 02.01.2021.
//

#pragma once

#include "attribute.h"

namespace ass {
    template<typename T>
    struct hovered: AttributeHelper<hovered<T>> {
        bool isPossiblyApplicable(AView* view) override {
            return dynamic_cast<T*>(view) != nullptr;
        }

        bool isStateApplicable(AView* view) override {
            return view->isMouseHover();
        }

        void setupConnections(AView* view, const _<AAssHelper>& helper) override {
            view->hoveredState.clearAllConnectionsWith(helper.get());
            AObject::connect(view->hoveredState, slot(helper)::onInvalidateStateAss);
        }
    };
}