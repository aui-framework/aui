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
// Created by dervisdev on 1/26/2023.
//

#include "ScrollbarAppearance.h"
#include <AUI/View/AScrollArea.h>

void ass::prop::Property<ass::ScrollbarAppearance>::applyFor(AView* view) {
    if (auto scrollArea = _cast<AScrollArea>(aui::ptr::shared_from_this(view))) {
        scrollArea->setScrollbarAppearance(mInfo);
    }
}
