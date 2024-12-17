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
// Created by alex2 on 04.09.2020.
//

#pragma once


#include "AListModelIndex.h"
#include "AListModelRange.h"
#include <AUI/Common/ASet.h>
#include <AUI/Common/AVector.h>
#include <cassert>

template<typename T> class IListModel;

template <typename T>
class AListModelSelection {
private:
    ASet<AListModelIndex> mIndices;
    _<IListModel<T>> mModel;

public:
    AListModelSelection() = default;
    AListModelSelection(const ASet<AListModelIndex>& indices, _<IListModel<T>> model) : mIndices(indices),
                                                                                    mModel(std::move(model)) {}

    class Iterator {
    private:
        ASet<AListModelIndex>::iterator mIterator;
        _<IListModel<T>> mModel;


    public:
        Iterator(const ASet<AListModelIndex>::iterator& iterator, _<IListModel<T>> model):
            mIterator(iterator), mModel(std::move(model)) {}


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
            AUI_ASSERT(mModel == other.mModel);
            return mIterator != other.mIterator;
        }

        bool operator==(const Iterator& other) const {
            AUI_ASSERT(mModel == other.mModel);
            return mIterator == other.mIterator;
        }

        [[nodiscard]]
        const AListModelIndex& getIndex() const {
            return *mIterator;
        }
    };

    AListModelSelection<T>::Iterator begin() const {
        return {mIndices.begin(), mModel};
    }
    AListModelSelection<T>::Iterator end() const {
        return {mIndices.end(), mModel};
    }

    [[nodiscard]] size_t size() const {
        return mIndices.size();
    }
    [[nodiscard]] bool empty() const {
        return mIndices.empty();
    }

    const _<IListModel<T>>& getModel() const {
        return mModel;
    }

    const ASet<AListModelIndex>& getIndices() const {
        return mIndices;
    }

    AListModelIndex first() {
        AUI_ASSERTX(mIndices.begin() != mIndices.end(), "selection model is empty");
        return *mIndices.begin();
    }

    AVector<AListModelRange<T>> ranges() const noexcept;
};

#include "IListModel.h"

template<typename T>
AVector<AListModelRange<T>> AListModelSelection<T>::ranges() const noexcept {
    return mModel->rangesIncluding([&](size_t i) {
        return mIndices.contains(i);
    });
}
