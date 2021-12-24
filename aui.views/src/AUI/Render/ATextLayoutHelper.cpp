//
// Created by Alex2772 on 11/28/2021.
//

#include <AUI/Traits/algorithms.h>
#include "ATextLayoutHelper.h"

size_t ATextLayoutHelper::xToIndex(int x) const {
    if (mSymbols.empty()) return -1;
    return xToIndex(mSymbols.first(), x);
}

size_t ATextLayoutHelper::xToIndex(const AVector<Symbol>& line, int pos) {
    // perform binary search in order to find index
    if (line.empty()) return 0;
    if (pos < 0) return 0;
    if (pos >= line.last().position.x) return line.last().index + 1;


    auto it = aui::binary_search(line.begin(), line.end(), [&](const AVector<Symbol>::const_iterator& it) {
        int posCurrent = it->position.x;
        if (posCurrent <= pos) {
            // check if the element is the last
            if (it + 1 == line.end()) return 0;
            int posNext = (it + 1)->position.x;
            if (pos < posNext) {
                return 0;
            }
        }
        if (posCurrent < pos) return 1;
        return -1;
    });
    if (it != line.end()) {
        return it->index;
    }
    return 0;
}

size_t ATextLayoutHelper::posToIndexFixedLineHeight(const glm::ivec2& position, const AFontStyle& font) const {
    if (mSymbols.empty()) return 0;
    size_t lineIndex = position.y / font.getLineHeight();
    if (lineIndex >= mSymbols.size()) {
        if (mSymbols.last().empty()) return 0;
        return mSymbols.last().last().index + 1;
    }
    return xToIndex(mSymbols[lineIndex], position.x);
}
