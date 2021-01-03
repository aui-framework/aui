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

namespace ass {

    namespace detail {
        struct ClassOf : virtual ISubSelector {
        private:
            AVector<AString> mClasses;
            
        public:
            ClassOf(const AVector<AString>& classes) : mClasses(classes) {}
            ClassOf(const AString& clazz) : mClasses({clazz}) {}

            bool isPossiblyApplicable(AView* view) override {
                for (auto& v : mClasses) {
                    if (view->getCssNames().contains(v))
                        return true;
                }
                return false;
            }

        };
    }

    struct class_of: detail::ClassOf, AttributeHelper<class_of> {
    public:
        class_of(const AVector<AString>& classes) : ClassOf(classes) {}
        class_of(const AString& clazz) : ClassOf(clazz) {}

        using hover = ass::hovered<detail::ClassOf>;
        using active = ass::active<detail::ClassOf>;
    };
}