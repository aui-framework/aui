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

#include "ATextField.h"
#include <AUI/Action/AMenu.h>


ATextField::ATextField()
{

    addAssName(".input-field");
}

bool ATextField::isValidText(std::u32string_view text) {
    return true;
}

ATextField::~ATextField() {

}

