#pragma once

#include <AUI/Platform/ACursor.h>
#include <AUI/Util/AMetric.h>
#include "IDeclaration.h"

namespace ass {
    namespace decl {
        template<>
        struct Declaration<AView::Overflow>: IDeclarationBase {
        private:
            AView::Overflow mInfo;

        public:
            Declaration(const AView::Overflow& info) : mInfo(info) {

            }

            void applyFor(AView* view) override;
        };
    }
}
