//
// Created by alex2 on 04.09.2020.
//

#pragma once


#include "AModelIndex.h"
#include <AUI/Common/ASet.h>

template<typename T> class IListModel;

template <typename T>
class AModelSelection {
private:
    ASet<AModelIndex> mIndices;
    IListModel<T>* mModel;

public:
    AModelSelection() = default;
    AModelSelection(const ASet<AModelIndex>& indices, IListModel<T>* model) : mIndices(indices),
                                                                              mModel(model) {}

    class Iterator {
    private:
        ASet<AModelIndex>::iterator mIterator;
        IListModel<T>* mModel;


    public:
        Iterator(const ASet<AModelIndex>::iterator& iterator, IListModel<T>* model):
            mIterator(iterator), mModel(model) {}

        T operator*() {
            return mModel->listItemAt(*mIterator);
        }

        Iterator& operator++() {
            ++mIterator;
            return *this;
        }

        bool operator!=(const Iterator& other) const {
            assert(mModel == other.mModel);
            return mIterator != other.mIterator;
        }

        bool operator==(const Iterator& other) const {
            assert(mModel == other.mModel);
            return mIterator == other.mIterator;
        }
    };

    AModelSelection<T>::Iterator begin() const {
        return mIndices.begin();
    }
    AModelSelection<T>::Iterator end() const {
        return mIndices.end();
    }

    [[nodiscard]] size_t size() const {
        return mIndices.size();
    }
    [[nodiscard]] bool empty() const {
        return mIndices.empty();
    }

    IListModel<T>* getModel() const {
        return mModel;
    }
};

#include "IListModel.h"