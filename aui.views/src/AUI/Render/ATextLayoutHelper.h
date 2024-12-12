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

/**
 * @brief Helps mapping prerendered string with positions.
 */
class ATextLayoutHelper {
public:
    struct Boundary {
        glm::ivec2 position;
    };

    /**
     * @brief Single line of symbols
     * @details
     * Actual size of line is +1 larger than symbol count; to expose last character's right bondary
     */
    using Line = AVector<Boundary>;
    using Symbols = AVector<Line>;

private:
    Symbols mSymbols;

    static size_t xToIndex(const AVector<Boundary>& line, int pos);

public:
    ATextLayoutHelper() = default;
    explicit ATextLayoutHelper(Symbols symbols) : mSymbols(std::move(symbols)) {}

    void setSymbols(Symbols symbols) {
        mSymbols = std::move(symbols);
    }

    [[nodiscard]]
    AOptional<glm::ivec2> indexToPos(size_t line, size_t column);

    [[nodiscard]]
    size_t posToIndexFixedLineHeight(const glm::ivec2& position, const AFontStyle& font) const;
};