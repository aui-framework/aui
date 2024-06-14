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
