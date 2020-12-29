#pragma once

class AView;

namespace ass::decl {
    struct IDeclarationBase {
    public:
        virtual void applyFor(AView* view) = 0;
    };
    template<typename DeclarationStruct>
    struct Declaration: IDeclarationBase {
    protected:
        DeclarationStruct mDeclarationStruct;
    public:
        Declaration(const DeclarationStruct& data): mDeclarationStruct(data) {}
    };
}

#include "AUI/View/AView.h"