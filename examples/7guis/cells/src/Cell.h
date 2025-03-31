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

#include <AUI/Common/AString.h>
#include <AUI/Common/AProperty.h>
#include "Formula.h"

class Spreadsheet;

struct Cell {
private:
    formula::Precompiled precompile();
    APropertyPrecomputed<formula::Precompiled> expressionPrecompiled = [&] { return precompile(); };
    formula::Value evaluate();

public:
    Spreadsheet* spreadsheet = nullptr;
    AProperty<AString> expression;
    APropertyPrecomputed<formula::Value> value = [&] { return (*expressionPrecompiled)(*spreadsheet); };

    static AString columnName(unsigned index);
    static AString rowName(unsigned index);

    static constexpr auto UNDEFINED = std::numeric_limits<unsigned>::max();

    static glm::uvec2 fromName(const AString& name);
};
