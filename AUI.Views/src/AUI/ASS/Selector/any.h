//
// Created by alex2 on 02.01.2021.
//

#pragma once

#include "attribute.h"
#include <AUI/Util/kAUI.h>
#include <AUI/View/AView.h>
#include <AUI/ASS/AAssHelper.h>

#include "hovered.h"
#include "active.h"
#include "focus.h"

namespace ass {

    namespace detail {
        template<typename T>
        struct Type : virtual ISubSelector {
        public:
            bool isPossiblyApplicable(AView* view) override {
                return dynamic_cast<T*>(view) != nullptr;
            }

        };
    }

    template<typename T>
    struct any: detail::Type<T>, AttributeHelper<any<T>> {
    public:

        using hover = ass::hovered<detail::Type<T>>;
        using active = ass::active<detail::Type<T>>;
        using focus = ass::active<detail::Type<T>>;
    };
}