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

#include "AForEachUI.h"

void AForEachUIBase::setModelImpl(_<IListModel<_<AView>>> model) {
    if (mViewsModel) {
        mViewsModel->dataInserted.clearAllOutgoingConnectionsWith(this);
        mViewsModel->dataChanged.clearAllOutgoingConnectionsWith(this);
        mViewsModel->dataRemoved.clearAllOutgoingConnectionsWith(this);
    }
    mViewsModel = std::move(model);

    if (!mViewsModel) {
        return;
    }
    for (size_t i = 0; i < mViewsModel->listSize(); ++i) {
        addView(mViewsModel->listItemAt(i));
    }

    connect(mViewsModel->dataInserted, this, [&](const AListModelRange<_<AView>>& data) {
        for (const auto& row : data) {
            addView(row.getIndex().getRow(), row.get());
        }
    });
    connect(mViewsModel->dataChanged, this, [&](const AListModelRange<_<AView>>& data) {
        for (const auto& row : data) {
            // TODO optimize
            removeView(row.getIndex().getRow());
            addView(row.getIndex().getRow(), row.get());
        }
    });
    connect(mViewsModel->dataRemoved, this, [&](const AListModelRange<_<AView>>& data) {
        for (const auto& row : data) {
            removeView(data.begin().getIndex().getRow());
        }
    });
}
