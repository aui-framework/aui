// AUI Framework - Declarative UI toolkit for modern C++17
// Copyright (C) 2020-2023 Alex2772
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

//
// Created by alex2 on 29.12.2020.
//

#include <AUI/Render/RenderHints.h>
#include "BackgroundGradient.h"
#include "IDeclaration.h"



void ass::decl::Declaration<ass::BackgroundGradient>::renderFor(AView* view) {
    if (mInfo.null) { return; }
    RenderHints::PushColor x;

    ABrush brush;
    if (mInfo.direction == ALayoutDirection::VERTICAL) {
        brush = ALinearGradientBrush{mInfo.topLeftColor,
                                     mInfo.topLeftColor,
                                     mInfo.bottomRightColor,
                                     mInfo.bottomRightColor};
    } else {
        brush = ALinearGradientBrush{mInfo.topLeftColor,
                                     mInfo.bottomRightColor,
                                     mInfo.topLeftColor,
                                     mInfo.bottomRightColor};
    }
    if (view->getBorderRadius() > 0) {
        Render::roundedRectAntialiased(brush, {0, 0}, view->getSize(), view->getBorderRadius());
    } else  {
        Render::rect(brush, {0, 0}, view->getSize());
    }
    IDeclarationBase::renderFor(view);
}

ass::decl::DeclarationSlot ass::decl::Declaration<ass::BackgroundGradient>::getDeclarationSlot() const {
    return ass::decl::DeclarationSlot::BACKGROUND_SOLID;
}


