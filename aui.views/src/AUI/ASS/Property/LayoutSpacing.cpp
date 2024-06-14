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
// Created by alex2772 on 1/3/21.
//

#include "LayoutSpacing.h"
#include <AUI/View/AViewContainer.h>


void ass::prop::Property<ass::LayoutSpacing>::applyFor(AView* view) {
    auto container = dynamic_cast<AViewContainer*>(view);
    if (container) {
        if (auto l = container->getLayout()) {
            l->setSpacing(mInfo.spacing);
        }
    }
}