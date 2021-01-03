//
// Created by alex2 on 02.01.2021.
//

#pragma once

#include "attribute.h"

namespace ass {
    template<typename Base>
    struct hovered: Base, AttributeHelper<hovered<Base>> {
        template<typename... Args>
        hovered(Args&&... args):
            Base(std::forward<Args>(args)...)
        {

        }

        bool isStateApplicable(AView* view) override {
            return Base::isStateApplicable(view) && view->isMouseHover();
        }

        void setupConnections(AView* view, const _<AAssHelper>& helper) override {
            Base::setupConnections(view, helper);
            view->hoveredState.clearAllConnectionsWith(helper.get());
            AObject::connect(view->hoveredState, slot(helper)::onInvalidateStateAss);
        }
    };
}