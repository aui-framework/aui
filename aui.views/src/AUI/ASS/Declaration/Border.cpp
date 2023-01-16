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
// Created by alex2 on 01.01.2021.
//

#include "Border.h"
#include <AUI/Render/Render.h>
#include <AUI/Render/RenderHints.h>


void ass::decl::Declaration<ass::Border>::renderFor(AView* view) {
    /*
    if (view->getBorderRadius() < 0.1f) {
        Render::drawRectBorder(ASolidBrush{mInfo.color},
                               {0, 0},
                               view->getSize(),
                               mInfo.width);
    } else {
        */
    Render::rectBorder(ASolidBrush{mInfo.color},
                       {0, 0},
                       view->getSize(),
                       view->getBorderRadius(),
                       mInfo.width);
    //}
}

bool ass::decl::Declaration<ass::Border>::isNone() {
    return mInfo.width <= 0.001f;
}

ass::decl::DeclarationSlot ass::decl::Declaration<ass::Border>::getDeclarationSlot() const {
    return ass::decl::DeclarationSlot::BORDER;
}