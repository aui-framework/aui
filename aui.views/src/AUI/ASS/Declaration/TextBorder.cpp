// AUI Framework - Declarative UI toolkit for modern C++20
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

#include "TextBorder.h"
#include <AUI/View/AAbstractLabel.h>
#include <AUI/Render/RenderHints.h>

void ass::decl::Declaration<ass::TextBorder>::renderFor(AView* view) {
    if (auto label = dynamic_cast<AAbstractLabel*>(view)) {
        RenderHints::PushColor c;
        Render::setColor(mInfo.borderColor);

        {
            RenderHints::PushMatrix m;
            Render::translate({-1, 0});
            label->doRenderText();
        }
        {
            RenderHints::PushMatrix m;
            Render::translate({1, 0});
            label->doRenderText();
        }
        {
            RenderHints::PushMatrix m;
            Render::translate({0, -1});
            label->doRenderText();
        }
        {
            RenderHints::PushMatrix m;
            Render::translate({0, 1});
            label->doRenderText();
        }
    }
}
ass::decl::DeclarationSlot ass::decl::Declaration<ass::TextBorder>::getDeclarationSlot() const {
    return ass::decl::DeclarationSlot::TEXT_SHADOW;
}