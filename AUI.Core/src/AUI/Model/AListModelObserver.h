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
    explicit AListModelObserver(IListModelListener* listener) : mListener(listener) {}


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

            connect(mModel->dataInserted, this, [&](const AModelRange<AString>& data) {
                for (const auto& row : data) {
                    mListener->insertItem(row.getIndex().getRow(), row.get());
                }
                mListener->onDataCountChanged();
                mListener->onDataChanged();
            });
            connect(mModel->dataChanged, this, [&](const AModelRange<AString>& data) {
                for (const auto& row : data) {
                    mListener->updateItem(row.getIndex().getRow(), row.get());
                }
                mListener->onDataChanged();
            });
            connect(mModel->dataRemoved, this, [&](const AModelRange<AString>& data) {
                for (const auto& row : data) {
                    mListener->removeItem(row.getIndex().getRow());
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
