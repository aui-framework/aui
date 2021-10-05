/**
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
 */

//
// Created by alex2 on 04.09.2020.
//

#pragma once


#include "AModelIndex.h"
#include <AUI/Common/ASet.h>
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
};

#include "IListModel.h"