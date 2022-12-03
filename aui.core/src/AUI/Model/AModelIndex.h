// AUI Framework - Declarative UI toolkit for modern C++17
// Copyright (C) 2020-2022 Alex2772
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

#pragma once

#include <cstddef>
#include <cstdint>
#include <tuple>
#include <ostream>

class AModelIndex
{
private:
    std::size_t mRow = -1;
    std::size_t mColumn = -1;

public:
    AModelIndex(std::size_t row, std::size_t column)
        : mRow(row),
          mColumn(column)
    {
    }

    AModelIndex(std::size_t row)
        : mRow(row)
    {
    }

    AModelIndex() = default;

    std::size_t getRow() const
    {
        return mRow;
    }

    std::size_t getColumn() const
    {
        return mColumn;
    }

    bool operator==(const AModelIndex& rhs) const {
        return std::tie(mRow, mColumn) == std::tie(rhs.mRow, rhs.mColumn);
    }

    bool operator!=(const AModelIndex& rhs) const {
        return !(rhs == *this);
    }

    inline bool operator<(const AModelIndex& other) const {
        return hash() < other.hash();
    }

    [[nodiscard]] inline uint64_t hash() const {
        uint64_t hash = uint32_t(mRow);
        hash <<= 32u;
        hash |= uint32_t(mColumn);
        return hash;
    }

};

inline std::ostream& operator<<(std::ostream& o, const AModelIndex& index) {
    o << "{ " << index.getRow();
    if (index.getColumn() != -1) {
        o << ", " << index.getColumn();
    }
    o << " }";

    return o;
}
