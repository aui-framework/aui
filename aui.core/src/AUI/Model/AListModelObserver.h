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


    void setModel(const _<IListModel<T>>& model) {
        if (mModel) {
            mModel->dataInserted.clearAllConnectionsWith(this);
            mModel->dataChanged.clearAllConnectionsWith(this);
            mModel->dataRemoved.clearAllConnectionsWith(this);
        }
        mModel = model;

        if (model) {
            for (size_t i = 0; i < model->listSize(); ++i) {
                mListener->insertItem(i, model->listItemAt(i));
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
