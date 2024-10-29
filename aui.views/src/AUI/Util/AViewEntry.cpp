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
// Created by Alex2772 on 12/31/2021.
//

#include "AViewEntry.h"

glm::ivec2 AViewEntry::getSize() {
    if (!(mView->getVisibility() & Visibility::CONSUMES_SPACE)) {
        return {0, 0};
    }
    return {mView->getMinimumWidth() + mView->getMargin().horizontal(), mView->getMinimumHeight(
            ALayoutDirection::NONE) + mView->getMargin().vertical() };
}

void AViewEntry::setPosition(const glm::ivec2& position) {
    mView->setGeometry(position + glm::ivec2{mView->getMargin().left, mView->getMargin().top},
                       mView->getMinimumSize());

}

Float AViewEntry::getFloat() const {
    return Float::NONE;
}

AViewEntry::~AViewEntry() {

}
