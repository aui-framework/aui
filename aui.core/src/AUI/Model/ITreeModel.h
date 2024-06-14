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
// Created by alex2 on 7/1/2021.
//


#pragma once

#include <AUI/Common/ASignal.h>
#include <any>
#include <utility>
#include "AListModelRange.h"
#include "AUI/Common/AException.h"
#include "AUI/Common/AObject.h"
#include "AUI/Traits/concepts.h"
#include <AUI/Model/ATreeModelIndex.h>

/**
 * @brief Tree model.
 * @tparam T itemAt value (typically AString)
 * @details
 * Tree model representation, used for ATreeView.
 */
template<typename T>
class ITreeModel: public AObject
{
public:
    virtual ~ITreeModel() = default;

    /**
     * @brief Root vertex.
     * @details
     * ATreeView never calls itemAt() on root vertex.
     */
    virtual ATreeModelIndex root() = 0;

    /**
     * @brief Count of children of the vertex.
     */
    virtual size_t childrenCount(const ATreeModelIndex& vertex) = 0;

    /**
     * @brief value representation, used by ATreeView.
     */
    virtual T itemAt(const ATreeModelIndex& index) = 0;

    /**
     * @brief Creates valid ATreeModelIndex of the child.
     * @param row child row (index)
     * @param column child column (typically 0)
     * @param vertex vertex
     */
    virtual ATreeModelIndex indexOfChild(size_t row, size_t column, const ATreeModelIndex& vertex) = 0;

    /**
     * @brief Creates valid index of parent.
     * @details
     * If vertex is root() vertex, the method should return empty ATreeModelIndex (ATreeModelIndex{}).
     */
    virtual ATreeModelIndex parent(const ATreeModelIndex& vertex) = 0;

    using stored_t = T;

    template<aui::predicate<ATreeModelIndex> Predicate>
    AOptional<ATreeModelIndex> find(const Predicate& predicate) {
        return visit(predicate, root());
    }

    template<aui::invocable<ATreeModelIndex> Callback>
    void forEachDirectChildOf(const ATreeModelIndex& vertex, Callback&& callback) {
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
    AOptional<ATreeModelIndex> visit(const Predicate& p, const ATreeModelIndex& vertex) {
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
