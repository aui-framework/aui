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
// Created by alex2 on 07.01.2021.
//


#include <AUI/View/AAbstractLabel.h>
#include "VerticalAlign.h"
#include "AUI/View/AText.h"
#include "AUI/View/ATextArea.h"

void ass::prop::Property<VerticalAlign>::applyFor(AView* view) {
    if (auto v = dynamic_cast<AAbstractLabel*>(view)) {
        v->setVerticalAlign(mInfo);
        return;
    }

    if (auto v = dynamic_cast<AText*>(view)) {
        v->setVerticalAlign(mInfo);
        return;
    }

    if (auto v = dynamic_cast<ATextArea*>(view)) {
        v->setVerticalAlign(mInfo);
        return;
    }
}