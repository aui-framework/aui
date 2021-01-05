#pragma once


#include <AUI/Platform/ACursor.h>
#include <AUI/Util/AMetric.h>
#include "IDeclaration.h"

namespace ass {
    struct LayoutSpacing {
        AMetric spacing;
    };

    namespace decl {
        template<>
        struct API_AUI_VIEWS Declaration<LayoutSpacing>: IDeclarationBase {
        private:
            LayoutSpacing mInfo;

        public:
            Declaration(const LayoutSpacing& info) : mInfo(info) {

            }

            void applyFor(AView* view) override;
        };
    }
}