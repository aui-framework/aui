#pragma once

#include <AUI/Common/ASignal.h>
#include "AUI/Common/AVariant.h"
#include "AModelRange.h"

template<typename T>
class IListModel
{
public:
	virtual ~IListModel() = default;

	virtual size_t listSize() = 0;
	virtual T listItemAt(const AModelIndex& index) = 0;

signals:
    /**
     * \brief Model data was changed
     */
    emits<AModelRange<T>> dataChanged;

    /**
     * \brief Model data was added
     */
    emits<AModelRange<T>> dataInserted;
};
