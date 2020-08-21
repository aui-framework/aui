#pragma once
#include "AUI/Common/AVariant.h"

class AModelIndex;

class API_AUI_CORE ITableModel
{
public:
	virtual ~ITableModel() = default;

	virtual size_t tableRows() = 0;
	virtual size_t tableColumns() = 0;
	virtual AVariant tableItemAt(const AModelIndex& index) = 0;
};
