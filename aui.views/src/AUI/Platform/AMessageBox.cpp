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
// Created by nelonn on 11/14/25.
//

#include "AMessageBox.h"

#include <AUI/Platform/APlatform.h>

namespace AMessageBox {
ResultButton show(AWindow* parent, const AString& title, const AString& message, Icon icon, Button b) {
    return APlatform::current().messageBoxShow(parent, title, message, icon, b);
}
}
