// AUI Framework - Declarative UI toolkit for modern C++17
// Copyright (C) 2020-2022 Alex2772
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
#include "BackgroundSolid.h"
#include "IDeclaration.h"


void ass::decl::Declaration<ass::BackgroundSolid>::renderFor(AView* view) {
    ASolidBrush brush = { mInfo.color };
    if (view->getBorderRadius() > 0) {
        Render::roundedRectAntialiased(brush,
                                       {0, 0},
                                       view->getSize(),
                                       view->getBorderRadius());
    } else  {
        Render::rect(brush,
                     {0, 0},
                     view->getSize());

    }

    IDeclarationBase::renderFor(view);
}

bool ass::decl::Declaration<ass::BackgroundSolid>::isNone() {
    return mInfo.color.isFullyTransparent();
}
ass::decl::DeclarationSlot ass::decl::Declaration<ass::BackgroundSolid>::getDeclarationSlot() const {
    return ass::decl::DeclarationSlot::BACKGROUND_SOLID;
}


