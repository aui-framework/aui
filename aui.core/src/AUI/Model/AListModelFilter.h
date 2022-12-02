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

