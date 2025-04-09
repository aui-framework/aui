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
// Created by alex2 on 07.01.2021.
//


#include "TransformOffset.h"
#include "AUI/Render/IRenderer.h"
#include <glm/gtc/matrix_transform.hpp>

void ass::prop::Property<ass::TransformOffset>::renderFor(AView* view, const ARenderContext& ctx) {
    ctx.render.setTransform(glm::translate(glm::mat4(1.f), glm::vec3{mInfo.offsetX, mInfo.offsetY, 0.0}));
}

ass::prop::PropertySlot ass::prop::Property<ass::TransformOffset>::getPropertySlot() const {
    return ass::prop::PropertySlot::TRANSFORM_OFFSET;
}

void ass::prop::Property<ass::TransformOffset>::updateInvalidPixelRect(ARect<int>& invalidRect) const {
    invalidRect.translate(glm::ivec2{mInfo.offsetX, mInfo.offsetY});
}
