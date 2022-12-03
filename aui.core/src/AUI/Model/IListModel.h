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

#include <AUI/Common/AVector.h>
#include <AUI/Common/ASignal.h>
#include <AUI/Model/AModelSelection.h>
#include "AModelRange.h"

template<typename T>
class IListModel
{
public:
    using value_type = T;
	virtual ~IListModel() = default;

	virtual size_t listSize() = 0;
	virtual T listItemAt(const AModelIndex& index) = 0;

	using stored_t = T;

	AModelRange<T> range(const AModelIndex& begin, const AModelIndex& end) {
        return AModelRange<T>(begin, end, this);
	}

	AModelRange<T> range(const AModelIndex& item) {
        return AModelRange<T>(item, {item.getRow() + 1}, this);
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
    AVector<AModelRange<T>> rangesIncluding(Filter&& filter) {
        AVector<AModelRange<T>> result;
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
    emits<AModelRange<T>> dataChanged;

    /**
     * @brief Model data was added
     */
    emits<AModelRange<T>> dataInserted;

    /**
     * @brief Model data about to remove
     */
    emits<AModelRange<T>> dataRemoved;
};
