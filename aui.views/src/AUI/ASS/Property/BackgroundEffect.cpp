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
// Created by alex2 on 29.12.2020.
//

#include <AUI/Render/RenderHints.h>
#include "BackgroundEffect.h"
#include "IProperty.h"


void ass::prop::Property<ass::BackgroundEffect>::renderFor(AView* view) {
    for (auto& e : mInfo.mEffects)
    {
        e->draw(view);
    }

    IPropertyBase::renderFor(view);
}

bool ass::prop::Property<ass::BackgroundEffect>::isNone() {
    return mInfo.mEffects.empty();
}
ass::prop::PropertySlot ass::prop::Property<ass::BackgroundEffect>::getPropertySlot() const {
    return ass::prop::PropertySlot::BACKGROUND_EFFECT;
}

