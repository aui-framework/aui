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
// Created by Alex2772 on 11/28/2021.
//

#include <AUI/Traits/algorithms.h>
#include "ATextLayoutHelper.h"
#include <limits>

size_t ATextLayoutHelper::xToIndex(const AVector<Boundary>& line, int pos) {
    pos += 2; // magic offset
    if (line.empty()) return 0;
    if (pos < 0) return 0;
    if (pos > line.last().position.x + 3) { // hardcoded char width
        return line.size() - 1; // last symbol boundary is the right border of the last character; hence -1
    }
    // perform binary search in order to find index
    auto it = aui::binary_search(line.begin(), line.end(), [&](const AVector<Boundary>::const_iterator& it) {
        int posCurrent = it->position.x;

        int leftBound = it == line.begin()
                ? 0
                : (posCurrent - (glm::max)((posCurrent - (it - 1)->position.x) / 2, 2) - 1);

        if (pos < leftBound) return aui::BinarySearchResult::LEFT;

        int rightBound = it == (line.end() - 1)
                ? std::numeric_limits<int>::max()
                : posCurrent + (glm::max)(((it + 1)->position.x - posCurrent) / 2, 2); // magic offset

        if (pos >= rightBound) return aui::BinarySearchResult::RIGHT;

        return aui::BinarySearchResult::MATCH;
    });
    if (it != line.end()) {
        const auto MAX = line.size() - 1; // last symbol boundary is the right border of the last character; hence -1
        return glm::min(size_t(it - line.begin()), MAX);
    }
    return 0;
}

size_t ATextLayoutHelper::posToIndexFixedLineHeight(const glm::ivec2& position, const AFontStyle& font) const {
    if (mSymbols.empty()) return 0;
    size_t lineIndex = (glm::max)(position.y, 0) / font.getLineHeight();
    if (lineIndex >= mSymbols.size()) {
        if (mSymbols.last().empty()) return 0;
        auto& lastLine = mSymbols.last();
        return lastLine.size() - 1;
    }
    return xToIndex(mSymbols[lineIndex], position.x);
}

AOptional<glm::ivec2> ATextLayoutHelper::indexToPos(size_t line, size_t column) {
    if (mSymbols.size() <= line) {
        return std::nullopt;
    }
    const auto& l = mSymbols[line];
    if (l.size() <= column) {
        return std::nullopt;
    }
    return l[column].position;
}
