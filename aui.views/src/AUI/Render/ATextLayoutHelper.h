// AUI Framework - Declarative UI toolkit for modern C++17
// Copyright (C) 2020-2023 Alex2772
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

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