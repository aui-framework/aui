#pragma once

#include <AUI/Platform/ACursor.h>
#include <AUI/Util/AMetric.h>
#include "IDeclaration.h"

namespace ass {
    namespace decl {
        template<>
        struct API_AUI_VIEWS Declaration<ACursor>: IDeclarationBase {
        private:
            ACursor mInfo;

        public:
            Declaration(const ACursor& info) : mInfo(info) {

            }

            void applyFor(AView* view) override;
        };
    }
}
