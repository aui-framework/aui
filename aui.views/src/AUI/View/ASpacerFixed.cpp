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

#include "ASpacerFixed.h"

int ASpacerFixed::getContentMinimumWidth() {
    if (auto parent = getParent()) {
        if (const auto& layout = parent->getLayout()) {
            if (layout->getLayoutDirection() == ALayoutDirection::HORIZONTAL) {
                return int(mSpace.getValuePx());
            }
        }
    }
    return 0;
}

int ASpacerFixed::getContentMinimumHeight() {
    if (auto parent = getParent()) {
        if (const auto& layout = parent->getLayout()) {
            if (layout->getLayoutDirection() == ALayoutDirection::VERTICAL) {
                return int(mSpace.getValuePx());
            }
        }
    }
    return 0;
}

bool ASpacerFixed::consumesClick(const glm::ivec2& pos) {
    return false;
}

_<AView> declarative::SpacerFixed::operator()() {
    auto view = _new<ASpacerFixed>(0);
    space.bindTo(ASlotDef { AUI_SLOT(view.get())::setSpace });
    return view;
}
