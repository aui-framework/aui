// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2024 Alex2772 and Contributors
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
#include "AUI/Common/AObject.h"
#include "AUI/Common/AOptional.h"
#include <AUI/Model/ATreeModelIndex.h>
#include <variant>

/**
 * @brief Tree model.
 * @tparam T itemAt value (typically AString)
 * @details
 * ITreeModel is an interface to a tree data structure used for ATreeView.
 *
 * Basic implementation of the tree model is available in ATreeModel.
 */
template<typename T>
class ITreeModel: public AObject
{
public:
    virtual ~ITreeModel() = default;

    /**
     * @brief Count of children of the vertex.
     * @param vertex the vertex to determine the children count of
     */
    virtual size_t childrenCount(const ATreeModelIndexOrRoot& vertex) = 0;

    /**
     * @brief value representation, used by ATreeView.
     */
    virtual T itemAt(const ATreeModelIndex& index) = 0;

    /**
     * @brief Creates valid ATreeModelIndex of the child.
     * @param row child row (index)
     * @param column child column (typically 0)
     * @param parent vertex. std::nullopt indicates root vertex
     * @return The child of the parent index with specified row and column.
     */
    virtual ATreeModelIndex indexOfChild(size_t row, size_t column,
                                         const ATreeModelIndexOrRoot& vertex) = 0;

    /**
     * @brief Creates index of parent vertex of the specified vertex.
     */
    virtual ATreeModelIndexOrRoot parent(const ATreeModelIndex& vertex) = 0;

    using stored_t = T;

    template<aui::predicate<ATreeModelIndex> Predicate>
    AOptional<ATreeModelIndex> find(const Predicate& predicate) {
        return visit(predicate, ATreeModelIndex::ROOT);
    }

    template<aui::invocable<ATreeModelIndex> Callback>
    void forEachDirectChildOf(const ATreeModelIndexOrRoot& vertex, Callback callback) {
        auto s = childrenCount(vertex);
        for (decltype(s) i = 0; i < s; ++i) {
            callback(indexOfChild(i, 0, vertex));
        }
    }

signals:
    /**
     * @brief Model data was changed
     */
    emits<ATreeModelIndex> dataChanged;

    /**
     * @brief Model data was added
     */
    emits<ATreeModelIndex> dataInserted;

    /**
     * @brief Model data about to remove
     */
    emits<ATreeModelIndex> dataRemoved;

private:
    template<aui::predicate<ATreeModelIndex> Predicate>
    AOptional<ATreeModelIndex> visit(const Predicate& p, const ATreeModelIndexOrRoot& vertex) {
        if (auto index = std::get_if<ATreeModelIndex>(&vertex)) {
            if (p(*index)) {
                return *index;
            }
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
