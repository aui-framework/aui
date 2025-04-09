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
#include "TextTransform.h"

void ass::prop::Property<TextTransform>::applyFor(AView* view) {
    if (auto label = dynamic_cast<AAbstractLabel*>(view)) {
        label->setTextTransform(mInfo);
    }
}