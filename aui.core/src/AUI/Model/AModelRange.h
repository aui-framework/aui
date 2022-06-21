/*
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
