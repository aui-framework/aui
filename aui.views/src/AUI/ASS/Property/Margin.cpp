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
// Created by alex2 on 01.01.2021.
//

#include "Margin.h"

void ass::prop::Property<ass::Margin>::applyFor(AView* view) {
    auto margin = view->getMargin();

    mInfo.left.bindTo(margin.left);
    mInfo.top.bindTo(margin.top);
    mInfo.right.bindTo(margin.right);
    mInfo.bottom.bindTo(margin.bottom);

    view->setMargin(margin);
}

void ass::prop::Property<ass::Margin>::updateInvalidPixelRect(ARect<int>& invalidRect) const {
    invalidRect.p1.x -= mInfo.left.orDefault(0).getValuePx();
    invalidRect.p1.y -= mInfo.top.orDefault(0).getValuePx();
    invalidRect.p2.x -= mInfo.right.orDefault(0).getValuePx();
    invalidRect.p2.y -= mInfo.bottom.orDefault(0).getValuePx();
}
