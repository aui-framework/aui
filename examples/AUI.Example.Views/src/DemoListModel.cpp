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
// Created by alex2 on 13.04.2021.
//


#include "DemoListModel.h"

#include <AUI/Traits/strings.h>

size_t DemoListModel::listSize() {
    return mListSize;
}

AString DemoListModel::listItemAt(const AListModelIndex& index) {
    return "List item #{}"_format(index.getRow() + 1);
}

void DemoListModel::addItem() {
    mListSize += 1;
    emit dataInserted(range({mListSize - 1}));
}

void DemoListModel::removeItem() {
    if (mListSize > 0) {
        mListSize -= 1;
        emit dataRemoved(range({mListSize}));
    }
}
