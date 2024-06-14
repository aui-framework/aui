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
// Created by alex2 on 07.01.2021.
//


#include "TransformScale.h"
#include "AUI/Render/ARender.h"
#include <glm/gtc/matrix_transform.hpp>

void ass::prop::Property<ass::TransformScale>::renderFor(AView* view) {
    auto pivot = view->getSize() / 2;
    glm::mat4 m(1.f);
    m = glm::translate(m, glm::vec3(pivot, 0.f));
    m = glm::scale(m, glm::vec3(mInfo.scale, 1.0f));
    m = glm::translate(m, glm::vec3(-pivot, 0.f));
    ARender::setTransform(m);
}

ass::prop::PropertySlot ass::prop::Property<ass::TransformScale>::getPropertySlot() const {
    return ass::prop::PropertySlot::TRANSFORM_SCALE;
}