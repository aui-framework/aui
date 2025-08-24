/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2025 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <AUI/Model/IListModel.h>
#include <AUI/Model/IMutableListModel.h>
#include <AUI/Common/AVector.h>

template<typename T, typename Filter>
class AListModelFilter: public IListModel<T> {
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

        AObject::connect(other->dataChanged, this, [&](const AListModelRange<T>& r){
            AUI_ASSERTX(0, "unimplemented");
        });
        AObject::connect(other->dataInserted, this, [&](const AListModelRange<T>& r){
            AUI_ASSERTX(0, "unimplemented");
        });
        AObject::connect(other->dataRemoved, this, [&](const AListModelRange<T>& r){
            AUI_ASSERTX(0, "unimplemented");
        });
    }


    ~AListModelFilter() override = default;

    size_t listSize() override {
        return mMapping.size();
    }

    T listItemAt(const AListModelIndex& index) override {
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
     * @details
     * If you guarantee, that filter's previous results is a subset of filter's new results, you should use this
     * function instead of [invalidate] because it's faster.
     */
    void lazyInvalidate() {
        auto ranges = this->rangesIncluding([&](size_t i) {
            return !mFilter(listItemAt(i));
        });
        int offset = 0;
        for (const AListModelRange<T>& r : ranges) {
            mMapping.erase(mMapping.begin() + r.getBegin().getRow() - offset, mMapping.begin() + r.getEnd().getRow()) - offset;
            offset += r.getEnd().getRow() - r.getBegin().getRow();
        }
    }
};

namespace AModels {
    template <typename Container, typename Filter>
    auto filter(const _<Container>& other, Filter&& filter) -> _<AListModelFilter<typename Container::value_type, Filter>> {
        return aui::ptr::manage_shared(new AListModelFilter<typename Container::value_type, Filter>(other, std::forward<Filter>(filter)));
    }
}

