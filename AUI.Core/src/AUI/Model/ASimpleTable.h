#pragma once

#include <cassert>

#include "AModelIndex.h"
#include "ITableModel.h"
#include "AUI/Common/AVector.h"

class API_AUI_CORE ASimpleTable: public ITableModel
{
protected:
	size_t mRowCount;
	size_t mColumnCount;

	AVector<AVariant> mData;
	
public:
	ASimpleTable(size_t rowCount, size_t columnCount)
		: mRowCount(rowCount),
		  mColumnCount(columnCount)
	{
		mData.resize(rowCount * columnCount);
	}

	~ASimpleTable() override;
	size_t tableRows() override;
	size_t tableColumns() override;
	AVariant tableItemAt(const AModelIndex& index) override;
	AVariant& itemAt(const AModelIndex& index)
	{
		size_t i = index.getRow() * mColumnCount + index.getColumn();
		assert(i < mData.size());
		return mData[i];
	}
};
