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

#pragma once

#include <AUI/Common/AObject.h>
#include "AUI/Util/ATokenizer.h"

namespace token {
struct Identifier {
    AString name;
};
struct Double {
    double value;
};
struct Semicolon {};   // ;
struct LPar {};        // (
struct RPar {};        // )
struct Colon {};       // :
struct Plus {};        // +
struct Minus {};       // -
struct Asterisk {};    // *
struct Slash {};       // /
struct LAngle {};      // <
struct RAngle {};      // >
struct StringLiteral {
    AString value;
};

using Any = std::variant<Identifier, Double, Semicolon, LPar, RPar, Colon, Plus, Minus, Asterisk, Slash, LAngle, RAngle, StringLiteral>;

AVector<token::Any> parse(aui::no_escape<ATokenizer> t);

}   // namespace token
