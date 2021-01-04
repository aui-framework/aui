#pragma once

#include <AUI/Platform/ACursor.h>
#include <AUI/Util/AMetric.h>
#include "IDeclaration.h"

namespace ass {
    namespace decl {
        template<>
        struct Declaration<Overflow>: IDeclarationBase {
        private:
            Overflow mInfo;

        public:
            Declaration(const Overflow& info) : mInfo(info) {

            }

            void applyFor(AView* view) override;
        };
    }
}
