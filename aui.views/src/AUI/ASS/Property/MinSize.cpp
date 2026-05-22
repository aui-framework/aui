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

#include "MinSize.h"

void ass::legacy::Property<ass::MinSize>::applyFor(AView* view) {
    view->setMinSize({
                               mInfo.width ? mInfo.width->getValuePx() : view->getMinSize().x,
                               mInfo.height ? mInfo.height->getValuePx() : view->getMinSize().y
                       });
}

namespace ass {
Modifier operator|(Modifier thiz, const MinSize& value) {
    return thiz.then([value](AView& view) {
        view.setMinSize({
            value.width ? value.width->getValuePx() : view.getMinSize().x,
            value.height ? value.height->getValuePx() : view.getMinSize().y
        });
    });
}
}   // namespace ass
