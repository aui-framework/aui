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
     * \brief В модели данные были изменены.
     */
    emits<AModelRange<T>> dataChanged;

    /**
     * \brief В модели данные были добавлены
     */
    emits<AModelRange<T>> dataInserted;
};
