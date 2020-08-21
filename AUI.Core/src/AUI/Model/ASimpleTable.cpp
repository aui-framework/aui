#include "ASimpleTable.h"

#include "AModelIndex.h"

ASimpleTable::~ASimpleTable()
{
}

size_t ASimpleTable::tableRows()
{
	return mRowCount;
}

size_t ASimpleTable::tableColumns()
{
	return mColumnCount;
}

AVariant ASimpleTable::tableItemAt(const AModelIndex& index)
{
	return itemAt(index);
}
