//
// Created by Alex2772 on 11/28/2021.
//

#include <AUI/Traits/algorithms.h>
#include "ATextLayoutHelper.h"
#include <limits>

size_t ATextLayoutHelper::xToIndex(int x) const {
    if (mSymbols.empty()) return -1;
    return xToIndex(mSymbols.first(), x);
}

size_t ATextLayoutHelper::xToIndex(const AVector<Symbol>& line, int pos) {
    pos += 2; // magic offset
    if (line.empty()) return 0;
    if (pos < 0) return 0;
    if (pos > line.last().position.x + 3) return line.size(); // hardcoded char width
    // perform binary search in order to find index
    auto it = aui::binary_search(line.begin(), line.end(), [&](const AVector<Symbol>::const_iterator& it) {
        int posCurrent = it->position.x;

        int leftBound = it == line.begin()
                ? 0
                : (posCurrent - (glm::max)((posCurrent - (it - 1)->position.x) / 2, 2) - 1);

        if (pos < leftBound) return -1;

        int rightBound = it == (line.end() - 1)
                ? std::numeric_limits<int>::max()
                : posCurrent + (glm::max)(((it + 1)->position.x - posCurrent) / 2, 2); // magic offset

        if (pos >= rightBound) return 1;

        return 0;
    });
    if (it != line.end()) {
        return it - line.begin();
    }
    return 0;
}

size_t ATextLayoutHelper::posToIndexFixedLineHeight(const glm::ivec2& position, const AFontStyle& font) const {
    if (mSymbols.empty()) return 0;
    size_t lineIndex = (glm::max)(position.y, 0) / font.getLineHeight();
    if (lineIndex >= mSymbols.size()) {
        if (mSymbols.last().empty()) return 0;
        auto& lastLine = mSymbols.last();
        return lastLine.size();
    }
    return xToIndex(mSymbols[lineIndex], position.x);
}
