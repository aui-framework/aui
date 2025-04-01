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

#pragma once

#include <AUI/Common/AObject.h>
#include "Tokens.h"
#include "Formula.h"

namespace ast {

class INodeVisitor {
public:
    virtual ~INodeVisitor() = default;

};

class INode {
public:
    INode() = default;
    virtual ~INode() = default;

    virtual formula::Value evaluate(const Spreadsheet& ctx) = 0;
};

_unique<INode> parseExpression(std::span<token::Any> tokens);

}