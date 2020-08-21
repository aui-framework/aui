#pragma once
#include "ISqlDriverRow.h"
#include "SqlTypes.h"
#include <AUI/Common/AVector.h>

class ISqlDriverResult
{
public:
	virtual ~ISqlDriverResult() = default;
	virtual const AVector<SqlColumn>& getColumns() = 0;
	virtual size_t rowCount() = 0;
	virtual _<ISqlDriverRow> begin() = 0;
	virtual _<ISqlDriverRow> next(const _<ISqlDriverRow>& previous) = 0;
};
