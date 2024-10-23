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
// Created by alex2 on 02.01.2021.
//

#include "TextColor.h"
#include "AUI/Font/IFontView.h"

void ass::prop::Property<ass::TextColor>::applyFor(AView* view) {
    AUI_NULLSAFE(dynamic_cast<IFontView*>(view))->setTextColor(mInfo.color);
}