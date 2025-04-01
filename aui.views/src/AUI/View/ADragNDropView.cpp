/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2025 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

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

void ADragNDropView::onPointerMove(glm::vec2 pos, const APointerMoveEvent& event) {
    AViewContainerBase::onPointerMove(pos, event);

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
