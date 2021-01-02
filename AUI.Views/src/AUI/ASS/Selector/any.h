//
// Created by alex2 on 02.01.2021.
//

#pragma once

#include "attribute.h"
#include <AUI/Util/kAUI.h>
#include <AUI/View/AView.h>
#include <AUI/ASS/AAssHelper.h>

namespace ass {

    template<typename T>
    struct any: AttributeHelper<any<T>> {
    public:
        bool isPossiblyApplicable(AView* view) override {
            return dynamic_cast<T*>(view) != nullptr;
        }

        struct hover: AttributeHelper<hover> {
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

        struct active: AttributeHelper<active> {
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
    };
}