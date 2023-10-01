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


#include "ATreeModelIndex.h"
#include <AUI/Common/SharedPtr.h>
#include <cassert>


template<typename T> class ITreeModel;

template <typename T>
class ATreeModelRange {
private:
    ATreeModelIndex mBegin;
    ATreeModelIndex mEnd;
    _<ITreeModel<T>> mModel;

public:
    ATreeModelRange() = default;
    ATreeModelRange(const ATreeModelIndex& begin, const ATreeModelIndex& end, _<ITreeModel<T>> model); 
    bool operator==(const ATreeModelRange& rhs) const {
        return std::tie(mBegin, mEnd, mModel) == std::tie(rhs.mBegin, rhs.mEnd, rhs.mModel);
    }

    class Iterator {
    private:
        ATreeModelIndex mIndex;
        _<ITreeModel<T>> mModel;


    public:
        Iterator(const ATreeModelIndex& index, _<ITreeModel<T>> model):
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

        [[nodiscard]] const ATreeModelIndex& getIndex() const {
            return mIndex;
        }
    };

    ATreeModelRange<T>::Iterator begin() const {
        return {mBegin, mModel};
    }
    ATreeModelRange<T>::Iterator end() const {
        return {mEnd, mModel};
    }

    [[nodiscard]] const ATreeModelIndex& getBegin() const {
        return mBegin;
    }

    [[nodiscard]] const ATreeModelIndex& getEnd() const {
        return mEnd;
    }

    const _<ITreeModel<T>>& getModel() const {
        return mModel;
    }
};


template<typename T>
inline std::ostream& operator<<(std::ostream& o, const ATreeModelRange<T>& range) {
    o << "[ " << range.getBegin() << "; " << range.getEnd() << " )";

    return o;
}

#include "ITreeModel.h"

template<typename T>
ATreeModelRange::ATreeModelRange(ATreeModelIndex begin, ATreeModelIndex end, _<ITreeModel<T>> model): mBegin(std::move(begin)),
                                                                                                      mEnd(std::move(end)),
                                                                                                      mModel(std::move(model)) {
    assert(mModel->parent(begin) == mModel->parent(end));
}
