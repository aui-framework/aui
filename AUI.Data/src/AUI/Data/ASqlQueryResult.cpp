#include "ASqlQueryResult.h"


ASqlQueryResult::Iterator::Iterator(const _<ISqlDriverResult>& sql): mResult(sql)
{
	if (mResult)
	{
		mRow = mResult->begin();
	}
}

ASqlQueryResult::Iterator ASqlQueryResult::begin() const
{
	return Iterator(mDriverInterface);
}

ASqlQueryResult::Iterator ASqlQueryResult::end() const
{
	return Iterator(nullptr);
}

size_t ASqlQueryResult::getRowCount() const
{
	return mDriverInterface->rowCount();
}

const AVector<SqlColumn>& ASqlQueryResult::getColumns() const
{
	return mDriverInterface->getColumns();
}
