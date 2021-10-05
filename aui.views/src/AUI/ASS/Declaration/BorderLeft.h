//
// Created by alex2 on 08.01.2021.
//

#pragma once

#include <AUI/ASS/Declaration/IDeclaration.h>
#include <AUI/Util/AMetric.h>

struct BorderLeft {
    AMetric width;
    AColor color;
};


namespace ass::decl {

    template<>
    struct API_AUI_VIEWS Declaration<BorderLeft>: IDeclarationBase {
    private:
        BorderLeft mInfo;

    public:
        Declaration(const BorderLeft& info) : mInfo(info) {

        }

        void renderFor(AView* view) override;

        bool isNone() override;

        DeclarationSlot getDeclarationSlot() const override;
    };
}

