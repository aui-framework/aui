//
// Created by alex2 on 02.01.2021.
//

#pragma once

#include "any.h"
#include <AUI/Common/AVector.h>

namespace ass {
    namespace detail {
        template<typename Base>
        struct any_attr_impl: Base {
        private:
            AVector <AString> mAttrs;

        public:
            any_attr_impl(const AVector <AString>& attrs) :
                    mAttrs(attrs) {

            }

            bool isStateApplicable(AView* view) override {
                if (!Base::isStateApplicable(view)) {
                    return false;
                }
                AMap <AString, AVariant> viewAttrs;
                view->getCustomCssAttributes(viewAttrs);
                for (auto& attr : mAttrs) {
                    if (!viewAttrs[attr].toBool()) {
                        return false;
                    }
                }
                return true;
            }
        };
    }

    template<typename T>
    struct any_attr: detail::any_attr_impl<any<T>> {
    public:
        any_attr(const AVector <AString>& attrs) :
                    detail::any_attr_impl<any<T>>(attrs) {

        }

        struct hover: detail::any_attr_impl<typename any<T>::hover> {
            hover(const AVector <AString>& attrs) :
                    detail::any_attr_impl<typename any<T>::hover>(attrs) {

            }
        };
        struct active: detail::any_attr_impl<typename any<T>::active> {
            active(const AVector <AString>& attrs) :
                    detail::any_attr_impl<typename any<T>::active>(attrs) {

            }
        };
    };
}