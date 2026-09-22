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

bool ASpacerFixed::consumesClick(const glm::ivec2& pos) {
    return false;
}

IInspectable::DebugInspectorInfo ASpacerFixed::debugInspectorInfo() {
    auto info = AView::debugInspectorInfo();
    info.push_back({"space", mSpace});
    auto parent = getParent();
    if (!parent) {
        info.push_back({"direction", "<no parent>"});
        return info;
    }
    const auto& layout = parent->getLayout();
    if (!layout) {
        info.push_back({"direction", "<no layout>"});
        return info;
    }
    info.push_back({"direction", layout->getLayoutDirection()});
    return info;
}

glm::ivec2 ASpacerFixed::onIntrinsicMeasure(AConstraints constraints) {
    if (auto parent = getParent()) {
        if (const auto& layout = parent->getLayout()) {
            switch (layout->getLayoutDirection()) {
            case ALayoutDirection::HORIZONTAL:
                return glm::ivec2(mSpace.getValuePx(), 0);
            case ALayoutDirection::VERTICAL:
                return glm::ivec2(0, mSpace.getValuePx());

            case ALayoutDirection::NONE:
                break;
            }
        }
    }
    return AView::onIntrinsicMeasure(constraints);
}
