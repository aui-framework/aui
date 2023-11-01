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

#pragma once


#include "AListModelIndex.h"
#include <AUI/Common/SharedPtr.h>
#include <cassert>


template<typename T> class IListModel;

template <typename T>
class AListModelRange {
private:
    AListModelIndex mBegin;
    AListModelIndex mEnd;
    _<IListModel<T>> mModel;

public:
    AListModelRange() = default;
    AListModelRange(const AListModelIndex& begin, const AListModelIndex& end, _<IListModel<T>> model) : mBegin(begin),
                                                                                                mEnd(end),
                                                                                                mModel(std::move(model)) {}

    bool operator==(const AListModelRange& rhs) const {
        return std::tie(mBegin, mEnd, mModel) == std::tie(rhs.mBegin, rhs.mEnd, rhs.mModel);
    }

    class Iterator {
    private:
        AListModelIndex mIndex;
        _<IListModel<T>> mModel;


    public:
        Iterator(const AListModelIndex& index, _<IListModel<T>> model):
                mIndex(index), mModel(std::move(model)) {}

        Iterator& operator*() {
            return *this;
        }

        auto& operator->() {
            return mModel->listItemAt(mIndex);
        }
        auto get() const {
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

        [[nodiscard]] const AListModelIndex& getIndex() const {
            return mIndex;
        }
    };

    AListModelRange<T>::Iterator begin() const {
        return {mBegin, mModel};
    }
    AListModelRange<T>::Iterator end() const {
        return {mEnd, mModel};
    }

    [[nodiscard]] const AListModelIndex& getBegin() const {
        return mBegin;
    }

    [[nodiscard]] const AListModelIndex& getEnd() const {
        return mEnd;
    }

    const _<IListModel<T>>& getModel() const {
        return mModel;
    }
};


template<typename T>
inline std::ostream& operator<<(std::ostream& o, const AListModelRange<T>& range) {
    o << "[ " << range.getBegin() << "; " << range.getEnd() << " )";

    return o;
}

#include "IListModel.h"
