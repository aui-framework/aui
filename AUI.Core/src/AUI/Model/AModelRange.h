#pragma once


#include "AModelIndex.h"
#include <AUI/Common/SharedPtr.h>
#include <cassert>


template<typename T> class IListModel;

template <typename T>
class AModelRange {
private:
    AModelIndex mBegin;
    AModelIndex mEnd;
    IListModel<T>* mModel;

public:
    AModelRange() = default;
    AModelRange(const AModelIndex& begin, const AModelIndex& end, IListModel<T>* model) : mBegin(begin),
                                                                                          mEnd(end),
                                                                                          mModel(model) {}

    class Iterator {
    private:
        AModelIndex mIndex;
        IListModel<T>* mModel;


    public:
        Iterator(const AModelIndex& index, IListModel<T>* model):
                mIndex(index), mModel(model) {}

        T operator*() {
            return mModel->listItemAt(mIndex);
        }

        Iterator& operator++() {
            mIndex = {mIndex.getRow() + 1, mIndex.getColumn()};
            return *this;
        }

        bool operator!=(const Iterator& other) const {
            assert(mModel == other.mModel);
            return mIndex.getRow() != other.mIndex.getRow();
        }

        bool operator==(const Iterator& other) const {
            assert(mModel == other.mModel);
            return mIndex.getRow() == other.mIndex.getRow();
        }
    };

    AModelRange<T>::Iterator begin() const {
        return {mBegin, mModel};
    }
    AModelRange<T>::Iterator end() const {
        return {mEnd, mModel};
    }

    [[nodiscard]] const AModelIndex& getBegin() const {
        return mBegin;
    }

    [[nodiscard]] const AModelIndex& getEnd() const {
        return mEnd;
    }

    IListModel<T>* getModel() const {
        return mModel;
    }
};

#include "IListModel.h"
