// AUI Framework - Declarative UI toolkit for modern C++20
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
// Created by alex2 on 7/1/2021.
//


#pragma once

#include <AUI/Common/ASignal.h>
#include <any>
#include <utility>
#include "AModelRange.h"

class ATreeIndex {
public:
    class Exception: public AException {
    public:
        using AException::AException;
    };

    ATreeIndex() = default;

    /**
     * @param row row of the vertex relative to it's parent. If vertex is root, row should be 0.
     * @param column column of the vertex relative to it's parent. If vertex is root, column should be 0.
     * @param userdata the data stored in this vertex.
     */
    explicit ATreeIndex(std::size_t row, std::size_t column, std::any userdata) noexcept:
        mRow(row),
        mColumn(column),
        mUserdata(std::move(userdata)) {}

    template<typename T>
    [[nodiscard]]
    T as() const {
        try {
            return std::any_cast<T>(mUserdata);
        } catch (...) {
            throw Exception(
                    "bad userdata type (expected {}, holds {})"_format(typeid(T).name(), mUserdata.type().name()));
        }
    }

    [[nodiscard]]
    bool hasValue() const noexcept {
        return mUserdata.has_value();
    }

    [[nodiscard]]
    std::size_t row() const {
        return mRow;
    }

    [[nodiscard]]
    std::size_t column() const {
        return mColumn;
    }

private:
    std::size_t mRow, mColumn;
    std::any mUserdata;
};

/**
 * @brief Tree model.
 * @tparam T itemAt value (typically AString)
 * @details
 * Tree model representation, used for ATreeView.
 */
template<typename T>
class ITreeModel
{
public:
    virtual ~ITreeModel() = default;

    /**
     * @brief Root vertex.
     * @details
     * ATreeView never calls itemAt() on root vertex.
     */
    virtual ATreeIndex root() = 0;

    /**
     * @brief Count of children of the vertex.
     */
    virtual size_t childrenCount(const ATreeIndex& vertex) = 0;

    /**
     * @brief value representation, used by ATreeView.
     */
    virtual T itemAt(const ATreeIndex& index) = 0;

    /**
     * @brief Creates valid ATreeIndex of the child.
     * @param row child row (index)
     * @param column child column (typically 0)
     * @param vertex vertex
     */
    virtual ATreeIndex indexOfChild(size_t row, size_t column, const ATreeIndex& vertex) = 0;

    /**
     * @brief Creates valid index of parent.
     * @details
     * If vertex is root() vertex, the method should return empty ATreeIndex (ATreeIndex{}).
     */
    virtual ATreeIndex parent(const ATreeIndex& vertex) = 0;

    using stored_t = T;

    template<aui::predicate<ATreeIndex> Predicate>
    AOptional<ATreeIndex> find(const Predicate& predicate) {
        return visit(predicate, root());
    }

signals:

private:
    template<aui::predicate<ATreeIndex> Predicate>
    AOptional<ATreeIndex> visit(const Predicate& p, const ATreeIndex& vertex) {
        if (p(vertex)) {
            return vertex;
        }
        std::size_t count = childrenCount(vertex);
        for (std::size_t i = 0; i < count; ++i) {
            auto child = indexOfChild(i, 0, vertex);
            if (auto v = visit(p, child)) {
                return v;
            }
        }
        return {};
    };
};
