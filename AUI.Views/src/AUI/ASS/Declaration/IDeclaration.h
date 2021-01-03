#pragma once

class AView;
class AAssHelper;

namespace ass::decl {
    enum class DeclarationSlot {
        NONE,

        SHADOW,
        BACKGROUND_SOLID,
        BACKGROUND_IMAGE,
        BORDER,

        COUNT,
    };

    struct IDeclarationBase {
    public:
        virtual void applyFor(AView* view) {};
        virtual void renderFor(AView* view) {};
        virtual bool isNone() { return false; }
        [[nodiscard]] virtual DeclarationSlot getDeclarationSlot() const {
            return DeclarationSlot::NONE;
        }
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
#include <AUI/ASS/unset.h>