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

AMinMaxSizes ASpacerFixed::onComputeIntrinsicMinMaxSizes(int height) {
    glm::ivec2 exactSize = {};
    if (auto parent = getParent()) {
        if (const auto& layout = parent->getLayout()) {
            if (layout->getLayoutDirection() == ALayoutDirection::HORIZONTAL) {
                exactSize.x = static_cast<int>(mSpace.getValuePx());
            }
            if (layout->getLayoutDirection() == ALayoutDirection::VERTICAL) {
                exactSize.y = static_cast<int>(mSpace.getValuePx());
            }
        }
    }
    return {
        .min = exactSize,
        .max = exactSize,
    };
}

bool ASpacerFixed::consumesClick(const glm::ivec2& pos) {
    return false;
}
