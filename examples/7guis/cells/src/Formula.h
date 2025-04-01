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

class Spreadsheet;

namespace formula {
    struct Range {
        glm::uvec2 from{}, to{};

        bool operator==(const Range&) const = default;
        bool operator!=(const Range&) const = default;
    };

    using Value = std::variant<std::nullopt_t, double, AString, Range>;
    using Precompiled = std::function<formula::Value(const Spreadsheet& spreadsheet)>;
    formula::Value evaluate(const Spreadsheet& spreadsheet, const AString& expression);
    Precompiled precompile(const AString& expression);
}