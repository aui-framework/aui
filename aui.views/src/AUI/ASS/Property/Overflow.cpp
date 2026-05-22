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
// Created by alex2772 on 1/3/21.
//

#include "Overflow.h"


void ass::legacy::Property<AOverflow>::applyFor(AView* view) {
    view->setOverflow(mInfo);
}


void ass::legacy::Property<AOverflowMask>::applyFor(AView* view) {
    view->setOverflowMask(mInfo);
}

namespace ass {
Modifier operator|(Modifier thiz, const AOverflow& value) {
    return thiz.then([value](AView& view) {
        view.setOverflow(value);
    });
}
}   // namespace ass

namespace ass {
Modifier operator|(Modifier thiz, const AOverflowMask& value) {
    return thiz.then([value](AView& view) {
        view.setOverflowMask(value);
    });
}
}   // namespace ass