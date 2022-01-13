#pragma once

#include <AUI/Common/AString.h>
#include "UIMatcher.h"

namespace By {
    API_AUI_UITESTS UIMatcher text(const AString& text);
    API_AUI_UITESTS UIMatcher name(const AString& name);

    template<typename T>
    UIMatcher type() {
        class TypeMatcher: public IMatcher {
        public:
            ~TypeMatcher() override = default;

            bool matches(const _<AView>& view) override {
                return typeid(*view.get()) == typeid(T);
            }
        };
        return { _new<TypeMatcher>() };
    }
}


