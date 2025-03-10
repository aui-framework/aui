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
// Created by alex2772 on 7/7/21.
//

#pragma once

#include <AUI/Common/AObject.h>
#include "IListModel.h"

template<typename T>
class AListModelObserver: public AObject {
public:
    class IListModelListener {
    public:
        virtual void insertItem(size_t at, const T& value) = 0;
        virtual void updateItem(size_t at, const T& value) = 0;
        virtual void removeItem(size_t at) = 0;

        virtual void onDataCountChanged() = 0;
        virtual void onDataChanged() = 0;
    };

private:
    _<IListModel<T>> mModel;
    IListModelListener* mListener;

public:
    explicit AListModelObserver(IListModelListener* listener) : mListener(listener) {
        setSlotsCallsOnlyOnMyThread(true);
    }


    void setModel(_<IListModel<T>> model) {
        if (mModel) {
            mModel->dataInserted.clearAllOutgoingConnectionsWith(this);
            mModel->dataChanged.clearAllOutgoingConnectionsWith(this);
            mModel->dataRemoved.clearAllOutgoingConnectionsWith(this);
        }
        mModel = std::move(model);

        if (mModel) {
            for (size_t i = 0; i < mModel->listSize(); ++i) {
                mListener->insertItem(i, mModel->listItemAt(i));
            }
            mListener->onDataCountChanged();
            mListener->onDataChanged();

            connect(mModel->dataInserted, this, [&](const AListModelRange<T>& data) {
                for (const auto& row : data) {
                    mListener->insertItem(row.getIndex().getRow(), row.get());
                }
                mListener->onDataCountChanged();
                mListener->onDataChanged();
            });
            connect(mModel->dataChanged, this, [&](const AListModelRange<T>& data) {
                for (const auto& row : data) {
                    mListener->updateItem(row.getIndex().getRow(), row.get());
                }
                mListener->onDataChanged();
            });
            connect(mModel->dataRemoved, this, [&](const AListModelRange<T>& data) {
                for (const auto& row : data) {
                    mListener->removeItem(data.begin().getIndex().getRow());
                }
                mListener->onDataCountChanged();
                mListener->onDataChanged();
            });
        }
    }

    [[nodiscard]]
    const _<IListModel<T>>& getModel() const {
        return mModel;
    }
};
