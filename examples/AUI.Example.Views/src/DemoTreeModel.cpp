/*
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
 */

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
