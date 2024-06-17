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
// Created by Alex2772 on 11/28/2021.
//

#pragma once

#include <AUI/Common/AVector.h>
#include <AUI/Font/AFontStyle.h>
#include <glm/glm.hpp>

class ATextLayoutHelper {
public:
    struct Symbol {
        glm::ivec2 position;
    };

    using Line = AVector<Symbol>;
    using Symbols = AVector<Line>;

private:
    Symbols mSymbols;

    static size_t xToIndex(const AVector<Symbol>& line, int pos);

public:
    ATextLayoutHelper() = default;
    explicit ATextLayoutHelper(Symbols symbols) : mSymbols(std::move(symbols)) {}

    void setSymbols(Symbols symbols) {
        mSymbols = std::move(symbols);
    }

    [[nodiscard]]
    size_t xToIndex(int x) const;
    size_t posToIndexFixedLineHeight(const glm::ivec2& position, const AFontStyle& font) const;
};