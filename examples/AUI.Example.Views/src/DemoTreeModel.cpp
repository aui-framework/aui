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

//
// Created by alex2 on 7/1/2021.
//


#include "DemoTreeModel.h"
#include <AUI/Logging/ALogger.h>

enum Type {
    TYPE_ROOT,
    TYPE_4ITEM,
    TYPE_ZERO_CHILD,
};

size_t DemoTreeModel::childrenCount(const ATreeIndex& parent) {
    switch ((uintptr_t)parent.getUserData<void>()) {
        case TYPE_ROOT:
            return 3;
        case TYPE_4ITEM:
            return 4;
    }
    return 0;
}

AString DemoTreeModel::itemAt(const ATreeIndex& index) {
    if (((uintptr_t)index.getUserData<void>()) != TYPE_ROOT) {
        return "Sub item";
    }
    return "Root item #";
}

ATreeIndex DemoTreeModel::indexOfChild(size_t row, size_t column, const ATreeIndex& parent) {
    switch ((uintptr_t)parent.getUserData<void>()) {
        case TYPE_ROOT:
            if (row == 0) {
                return ATreeIndex{(void*)TYPE_4ITEM};
            }
            break;

    }
    return ATreeIndex{(void*)TYPE_ZERO_CHILD};
}

void *DemoTreeModel::rootUserData() {
    return (void*)TYPE_ROOT;
}
