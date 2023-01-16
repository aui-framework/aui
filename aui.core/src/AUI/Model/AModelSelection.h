// AUI Framework - Declarative UI toolkit for modern C++17
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
// Created by alex2 on 04.09.2020.
//

#pragma once


#include "AModelIndex.h"
#include "AModelRange.h"
#include <AUI/Common/ASet.h>
#include <AUI/Common/AVector.h>
#include <cassert>

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


        Iterator& operator*() {
            return *this;
        }

        auto& operator->() {
            return mModel->listItemAt(*mIterator);
        }
        auto get() const {
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

        [[nodiscard]]
        const AModelIndex& getIndex() const {
            return *mIterator;
        }
    };

    AModelSelection<T>::Iterator begin() const {
        return {mIndices.begin(), mModel};
    }
    AModelSelection<T>::Iterator end() const {
        return {mIndices.end(), mModel};
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

    const ASet<AModelIndex>& getIndices() const {
        return mIndices;
    }

    AModelIndex one() {
        assert(("selection model is empty" && mIndices.begin() != mIndices.end()));
        return *mIndices.begin();
    }

    AVector<AModelRange<T>> ranges() const noexcept;
};

#include "IListModel.h"

template<typename T>
AVector<AModelRange<T>> AModelSelection<T>::ranges() const noexcept {
    return mModel->rangesIncluding([&](size_t i) {
        return mIndices.contains(i);
    });
}
