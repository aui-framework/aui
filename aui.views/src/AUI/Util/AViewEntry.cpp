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
// Created by Alex2772 on 12/31/2021.
//

#include "AViewEntry.h"

glm::ivec2 AViewEntry::getSize() {
    if (!(mView->getVisibility() & Visibility::FLAG_CONSUME_SPACE)) {
        return {0, 0};
    }
    const auto preferred = mView->computeMinMaxSizes().max;
    return {
        preferred.x + mView->getMargin().horizontal(),
        preferred.y + mView->getMargin().vertical() };
}

void AViewEntry::setPosition(glm::ivec2 position) {
    Entry::setPosition(position);
    auto s = getSize();
    mView->layout(position + glm::ivec2{mView->getMargin().left, mView->getMargin().top},
                  s - mView->getMargin().occupiedSize());

}

AFloat AViewEntry::getFloat() const {
    return mView->getFloating();
}

AViewEntry::~AViewEntry() {

}
