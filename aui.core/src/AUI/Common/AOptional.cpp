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
// Created by alex2772 on 9/8/22.
//

#include "AOptional.h"
#include "AException.h"


// avoid of cycling includes is the reason why the throw implementation is dedicated to cpp file
void aui::impl::optional::throwException(const char* message) {
    throw AException(message);
}
