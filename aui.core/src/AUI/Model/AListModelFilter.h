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

#include <AUI/Model/IListModel.h>
#include <AUI/Model/IMutableListModel.h>
#include <AUI/Common/AVector.h>

template<typename T, typename Filter>
class AListModelFilter: public IListModel<T>, public AObject {
private:
    _<IListModel<T>> mOther;
    Filter mFilter;
    AVector<size_t> mMapping;

    void fill() {
        for (size_t i = 0; i < mOther->listSize(); ++i) {
            if (mFilter(mOther->listItemAt(i))) {
                mMapping.push_back(i);
            }
        }
    }
public:
    using value_type = T;

    explicit AListModelFilter(const _<IListModel<T>>& other, Filter&& adapter) :
            mOther(other),
            mFilter(std::forward<Filter>(adapter)) {
        fill();

        AObject::connect(other->dataChanged, this, [&](const AModelRange<T>& r){
            assert(("unimplemented" && 0));
        });
        AObject::connect(other->dataInserted, this, [&](const AModelRange<T>& r){
            assert(("unimplemented" && 0));
        });
        AObject::connect(other->dataRemoved, this, [&](const AModelRange<T>& r){
            assert(("unimplemented" && 0));
        });
    }


    ~AListModelFilter() override = default;

    size_t listSize() override {
        return mMapping.size();
    }

    T listItemAt(const AModelIndex& index) override {
        return mOther->listItemAt(mMapping[index.getRow()]);
    }

    /**
     * Removes all items and performs filtering again for all elements.
     */
    void invalidate() {
        size_t prevSize = mMapping.size();
        mMapping.clear();
        fill();

        size_t currentSize = mMapping.size();
        if (currentSize == prevSize) {
            emit this->dataChanged(this->range(0, currentSize));
        } else if (currentSize > prevSize) {
            emit this->dataChanged(this->range(0, prevSize));
            emit this->dataInserted(this->range(prevSize, currentSize));
        } else {
            emit this->dataChanged(this->range(0, currentSize));
            emit this->dataRemoved(this->range(currentSize, prevSize));
        }
    }

    /**
     * @brief Performs filtering for currently unfiltered elements.
     * @note If you guarantee, that filter's previous results is a subset of filter's new results, you should use this
     *       function instead of <a href="#invalidate">invalidate</a> because it's faster.
     */
    void lazyInvalidate() {
        auto ranges = this->rangesIncluding([&](size_t i) {
            return !mFilter(listItemAt(i));
        });
        int offset = 0;
        for (const AModelRange<T>& r : ranges) {
            mMapping.erase(mMapping.begin() + r.getBegin().getRow() - offset, mMapping.begin() + r.getEnd().getRow()) - offset;
            offset += r.getEnd().getRow() - r.getBegin().getRow();
        }
    }
};

namespace AModels {
    template <typename Container, typename Filter>
    auto filter(const _<Container>& other, Filter&& filter) -> _<AListModelFilter<typename Container::value_type, Filter>> {
        return aui::ptr::manage(new AListModelFilter<typename Container::value_type, Filter>(other, std::forward<Filter>(filter)));
    }
}

