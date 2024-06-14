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

#include <cstddef>
#include <cstdint>
#include <tuple>
#include <ostream>

class AListModelIndex
{
private:
    std::size_t mRow = -1;
    std::size_t mColumn = -1;

public:
    AListModelIndex(std::size_t row, std::size_t column)
        : mRow(row),
          mColumn(column)
    {
    }

    AListModelIndex(std::size_t row)
        : mRow(row)
    {
    }

    AListModelIndex() = default;

    std::size_t getRow() const
    {
        return mRow;
    }

    std::size_t getColumn() const
    {
        return mColumn;
    }

    bool operator==(const AListModelIndex& rhs) const {
        return std::tie(mRow, mColumn) == std::tie(rhs.mRow, rhs.mColumn);
    }

    bool operator!=(const AListModelIndex& rhs) const {
        return !(rhs == *this);
    }

    inline bool operator<(const AListModelIndex& other) const {
        return hash() < other.hash();
    }

    [[nodiscard]] inline uint64_t hash() const {
        uint64_t hash = uint32_t(mRow);
        hash <<= 32u;
        hash |= uint32_t(mColumn);
        return hash;
    }

};

inline std::ostream& operator<<(std::ostream& o, const AListModelIndex& index) {
    o << "{ " << index.getRow();
    if (index.getColumn() != -1) {
        o << ", " << index.getColumn();
    }
    o << " }";

    return o;
}
