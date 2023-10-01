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

#include <AUI/Common/AVector.h>
#include <AUI/Common/ASignal.h>
#include <AUI/Model/AListModelSelection.h>
#include <memory>
#include "AListModelRange.h"
#include "AUI/Common/AObject.h"

template<typename T>
class IListModel: public AObject 
{
public:
    using value_type = T;
	virtual ~IListModel() = default;

	virtual size_t listSize() = 0;
	virtual T listItemAt(const AListModelIndex& index) = 0;

	using stored_t = T;

	AListModelRange<T> range(const AListModelIndex& begin, const AListModelIndex& end) {
        return AListModelRange<T>(begin, end, std::dynamic_pointer_cast<IListModel<T>>(this->shared_from_this()));
	}

	AListModelRange<T> range(const AListModelIndex& item) {
        return AListModelRange<T>(item, {item.getRow() + 1}, std::dynamic_pointer_cast<IListModel<T>>(this->shared_from_this()));
	}


    AVector<T> toVector() noexcept {
        AVector<T> result;
        size_t size = listSize();
        result.reserve(size);
        for (size_t i = 0; i < size; ++i) {
            result << listItemAt(i);
        }
        return result;
    }

    template<typename Filter>
    AVector<AListModelRange<T>> rangesIncluding(Filter&& filter) {
        AVector<AListModelRange<T>> result;
        size_t currentBeginning = 0;
        size_t s = listSize();
        bool prevValue = false;
        bool currentValue;

        auto endSliceIfNecessary = [&](size_t i) {
            if (prevValue) {
                result << range(currentBeginning, i);
            }
        };
        for (size_t i = 0; i < s; ++i, prevValue = currentValue) {
            currentValue = filter(i);
            if (currentValue) {
                if (!prevValue) {
                    currentBeginning = i;
                }
            } else {
                endSliceIfNecessary(i);
            }
        }
        endSliceIfNecessary(listSize());
        return result;
    }

signals:
    /**
     * @brief Model data was changed
     */
    emits<AListModelRange<T>> dataChanged;

    /**
     * @brief Model data was added
     */
    emits<AListModelRange<T>> dataInserted;

    /**
     * @brief Model data about to remove
     */
    emits<AListModelRange<T>> dataRemoved;
};
