/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

//
// Created by alex2 on 29.12.2020.
//

#include <AUI/Render/RenderHints.h>
#include "ImageRendering.h"
#include "IProperty.h"
#include <AUI/ASS/AAssHelper.h>



void ass::prop::Property<ImageRendering>::renderFor(AView* view, const ARenderContext& ctx) {
    view->getAssHelper()->state.imageRendering = mInfo;
}

ass::prop::PropertySlot ass::prop::Property<ImageRendering>::getPropertySlot() const {
    return ass::prop::PropertySlot::IMAGE_RENDERING;
}


