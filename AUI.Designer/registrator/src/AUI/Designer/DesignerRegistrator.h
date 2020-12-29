#pragma once

#include <AUI/Common/AString.h>
#include <AUI/Common/SharedPtr.h>
#include <AUI/Common/AVector.h>
#include <AUI/Reflect/AClass.h>
#include <AUI/View/AView.h>

namespace aui::detail {
    struct API_AUI_DESIGNER_REGISTRATOR DesignerRegistrationBase {
    public:
        DesignerRegistrationBase() noexcept;
        virtual ~DesignerRegistrationBase() noexcept;
        virtual _<AView> instanciate() = 0;
        virtual AString name() = 0;

        static const AVector<DesignerRegistrationBase*>& getRegistrations();
    };

    template <typename V>
    struct DesignerRegistration: aui::detail::DesignerRegistrationBase {
    private:
    public:
        AString name() override {
            return AClass<V>::name();
        }
    };
}

#define PP_CAT(a, b) PP_CAT_I(a, b)
#define PP_CAT_I(a, b) PP_CAT_II(~, a ## b)
#define PP_CAT_II(p, res) res

#define DESIGNER_REGISTRATION(clazz, code) struct PP_CAT(clazz, Registrator): aui::detail::DesignerRegistration<clazz> code PP_CAT(clazz, RegistratorInst);