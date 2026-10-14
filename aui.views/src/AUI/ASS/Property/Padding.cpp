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
// Created by alex2 on 01.01.2021.
//

#include "Padding.h"

void ass::legacy::Property<ass::Padding>::applyFor(AView* view) {
    auto padding = view->getPadding();

    mInfo.left.bindTo(padding.left);
    mInfo.top.bindTo(padding.top);
    mInfo.right.bindTo(padding.right);
    mInfo.bottom.bindTo(padding.bottom);

    view->setPadding(padding);
}

namespace ass {
Modifier operator|(Modifier thiz, Padding value) {
    return thiz.then([value](AView& view) {
        auto padding = view.getPadding();
        value.left.bindTo(padding.left);
        value.top.bindTo(padding.top);
        value.right.bindTo(padding.right);
        value.bottom.bindTo(padding.bottom);
        view.setPadding(padding);
    });
}
}   // namespace ass
