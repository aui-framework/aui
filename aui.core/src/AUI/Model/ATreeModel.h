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

#include "AUI/Model/ATreeModelIndex.h"
#include "ITreeModel.h"


/**
 * @brief Basic implementation of ITreeModel.
 */
template<typename T>
class ATreeModel: public ITreeModel<T> {
private:
    struct Node {
        T value;
        AVector<_<Node>> children;
        _weak<Node> parent;
    };

public:
    struct Item {
        T value;
        AVector<Item> children;
    };

private:
    static _<Node> itemToNode(Item& item) {
        auto result = _new<Node>(Node {
            .value = std::move(item.value),
        });
        result->children = item.children.map([&](Item& item) {
            auto child = itemToNode(item);
            child->parent = result;
            return child;
        });

        return result;
    }

public:
    ATreeModel() = default;
    ATreeModel(AVector<Item> items): mChildren(items.map(itemToNode)) {

    }
    ~ATreeModel() override = default;


    size_t childrenCount(const ATreeModelIndexOrRoot& vertex) override {
        auto& container = vertex == ATreeModelIndex::ROOT ? mChildren : (*vertex).as<_<Node>>()->children;
        return container.size();
    }

     T itemAt(const ATreeModelIndex& index) override {
        return index.as<_<Node>>()->value;
     }

    ATreeModelIndex indexOfChild(size_t row, size_t column, const ATreeModelIndexOrRoot& vertex) override {
        auto& container = vertex == ATreeModelIndex::ROOT ? mChildren : (*vertex).as<_<Node>>()->children;
        return ATreeModelIndex(row, column, container[row]);
    }

    ATreeModelIndex makeIndex(_<Node> node) {
        auto parent = node->parent.lock();
        auto& children = parent ? parent->children : mChildren;
        return ATreeModelIndex(children.indexOf(node), 0, std::move(node));
    }

    ATreeModelIndexOrRoot parent(const ATreeModelIndex& vertex) override {
        if (auto p = vertex.as<_<Node>>()->parent.lock()) {
            return makeIndex(std::move(p));
        }
        return ATreeModelIndex::ROOT;
    }

private:

    AVector<_<Node>> mChildren;
};