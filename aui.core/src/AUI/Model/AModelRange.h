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

    bool operator==(const AModelRange& rhs) const {
        return std::tie(mBegin, mEnd, mModel) == std::tie(rhs.mBegin, rhs.mEnd, rhs.mModel);
    }

    class Iterator {
    private:
        AModelIndex mIndex;
        IListModel<T>* mModel;


    public:
        Iterator(const AModelIndex& index, IListModel<T>* model):
                mIndex(index), mModel(model) {}

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

        [[nodiscard]] const AModelIndex& getIndex() const {
            return mIndex;
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


template<typename T>
inline std::ostream& operator<<(std::ostream& o, const AModelRange<T>& range) {
    o << "[ " << range.getBegin() << "; " << range.getEnd() << " )";

    return o;
}

#include "IListModel.h"
