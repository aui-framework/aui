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

#include "MaxSize.h"

void ass::legacy::Property<ass::MaxSize>::applyFor(AView* view) {
    view->setMaxSize({
                               mInfo.width ? mInfo.width->getValuePx() : view->getMaxSize().x,
                               mInfo.height ? mInfo.height->getValuePx() : view->getMaxSize().y
                       });
}

namespace ass {
Modifier operator|(Modifier thiz, MaxSize value) {
    return thiz.then([value](AView& view) {
        view.setMaxSize({
            value.width ? value.width->getValuePx() : view.getMaxSize().x,
            value.height ? value.height->getValuePx() : view.getMaxSize().y
        });
    });
}
}   // namespace ass
