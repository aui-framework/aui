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
    };
}