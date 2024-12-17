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
// Created by Alex2772 on 1/27/2023.
//

#include "AWin32Event.h"

AWin32Event::AWin32Event(LPSECURITY_ATTRIBUTES securityAttributes, bool manualReset, bool initialState, LPCWSTR name):
        mEvent(CreateEvent(securityAttributes, manualReset, initialState, name)) {

}

AWin32Event::~AWin32Event() {
    CloseHandle(mEvent);
}
