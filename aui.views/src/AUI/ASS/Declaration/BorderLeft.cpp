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
// Created by alex2 on 08.01.2021.
//

#include <AUI/Render/RenderHints.h>
#include "BorderLeft.h"

void ass::decl::Declaration<ass::BorderLeft>::renderFor(AView* view) {
    RenderHints::PushColor x;
    int w = mInfo.width;

    Render::rect(ASolidBrush{mInfo.color},
                 {0, 0},
                 {w, view->getHeight()});

}

bool ass::decl::Declaration<ass::BorderLeft>::isNone() {
    return mInfo.width <= 0.001f;
}

ass::decl::DeclarationSlot ass::decl::Declaration<ass::BorderLeft>::getDeclarationSlot() const {
    return ass::decl::DeclarationSlot::BORDER;
}