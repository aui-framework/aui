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
// Created by Alex2772 on 9/15/2022.
//

#include "ADragNDropView.h"
#include "AUI/Util/UIBuildingHelpers.h"
#include "AUI/Platform/ADragNDrop.h"

using namespace declarative;

ADragNDropView::ADragNDropView() {
    setContents(Centered {
        Label { "d&d test" },
    });
}

void ADragNDropView::onPointerMove(glm::ivec2 pos, const APointerMoveEvent& event) {
    AViewContainer::onPointerMove(pos, event);

    if (AInput::isKeyDown(AInput::LBUTTON)) {
        if (!mDnDInProgress) {
            mDnDInProgress = true;
            ADragNDrop v;
            AMimedData data;
            data.setText("hello world!");
            v.setData(std::move(data));
            v.perform(getWindow());
        }
    } else {
        mDnDInProgress = false;
    }
}
