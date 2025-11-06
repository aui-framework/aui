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

#include "Cell.h"

AString Cell::columnName(unsigned int index) {
    return AString(char('A' + index));
}
AString Cell::rowName(unsigned int index) {
    return AString::number(index);
}

formula::Value Cell::evaluate() {
    return formula::evaluate(*spreadsheet, expression);
}

glm::uvec2 Cell::fromName(const AString& name) {
    glm::uvec2 out{UNDEFINED};
    auto it = name.begin();
    for (;it != name.end() && 'A' <= *it && *it <= 'Z'; ++it) {
        if (out.x == UNDEFINED) { out.x = 0; }
        out.x *= 26;
        out.x += *it - 'A';
    }
    for (;it != name.end() && '0' <= *it && *it <= '9'; ++it) {
        if (out.y == UNDEFINED) { out.y = 0; }
        out.y *= 10;
        out.y += *it - '0';
    }

    return out;
}

formula::Precompiled Cell::precompile() {
    return formula::precompile(expression);
}
