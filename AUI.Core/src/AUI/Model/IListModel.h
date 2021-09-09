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
 
 * Original code located at https://github.com/Alex2772/aui
 * =====================================================================================================================
 */

#pragma once

#include <AUI/Common/ASignal.h>
#include <AUI/Model/AModelSelection.h>
#include "AUI/Common/AVariant.h"
#include "AModelRange.h"

template<typename T>
class IListModel
{
public:
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

    virtual void removeItems(const AModelRange<T>& items) = 0;
    virtual void removeItems(const AModelSelection<T>& items) {
        for (auto& item : items) {
            removeItem(item.getIndex());
        }
    }
    virtual void removeItem(const AModelIndex& item) = 0;

signals:
    /**
     * \brief Model data was changed
     */
    emits<AModelRange<T>> dataChanged;

    /**
     * \brief Model data was added
     */
    emits<AModelRange<T>> dataInserted;

    /**
     * \brief Model data about to remove
     */
    emits<AModelRange<T>> dataRemoved;
};
