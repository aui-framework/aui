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
        size_t index;
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