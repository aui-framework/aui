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
// Created by Alex2772 on 12/7/2021.
//

#include "AFakeWindowInitializer.h"
#include "ABaseWindow.h"


void AFakeWindowInitializer::init(const IRenderingContext::Init& init) {
    IRenderingContext::init(init);
}

void AFakeWindowInitializer::destroyNativeWindow(ABaseWindow& window) {

}

void AFakeWindowInitializer::beginPaint(ABaseWindow& window) {

}

void AFakeWindowInitializer::endPaint(ABaseWindow& window) {

}

void AFakeWindowInitializer::beginResize(ABaseWindow& window) {

}
