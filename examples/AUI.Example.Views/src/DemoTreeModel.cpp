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


#include "DemoTreeModel.h"
#include "AUI/Common/AStringVector.h"
#include <AUI/Logging/ALogger.h>
#include <optional>


namespace {
struct Node {
    bool hasChildren;
};
}

ATreeModelIndex DemoTreeModel::root() {
    return ATreeModelIndex(0, 0, Node { .hasChildren = true });
}

size_t DemoTreeModel::childrenCount(const ATreeModelIndex& vertex) {
    auto node = vertex.as<Node>();
    if (node.hasChildren) {
        return 3;
    }
    return 0;
}

AString DemoTreeModel::itemAt(const ATreeModelIndex& index) {
    return AString::number(index.row());
}

ATreeModelIndex DemoTreeModel::indexOfChild(size_t row, size_t column, const ATreeModelIndex& vertex) {
    return ATreeModelIndex(row, column, Node { .hasChildren = false });
}

ATreeModelIndex DemoTreeModel::parent(const ATreeModelIndex& vertex) {
    return root(); 
}
