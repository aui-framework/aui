#pragma once

#include <AUI/Common/AString.h>
#include "Matcher.h"

namespace By {
    API_AUI_UITESTS Matcher text(const AString& text);
    API_AUI_UITESTS Matcher name(const AString& name);

    template<typename T>
    Matcher type() {
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


