#include "ASqlTable.h"

ASqlTable::ASqlTable(_<ASqlQueryResult> result):
	ASimpleTable(0, result->getColumns().size())
{
	mData.reserve(1000 * result->getColumns().size());
	
	size_t rowCounter = 0;
	for (auto& r : *result)
	{
		for (size_t columnCounter = 0; columnCounter < result->getColumns().size(); ++columnCounter) {
			mData.push_back(r.getValue(columnCounter));
		}
		++rowCounter;
	}
	mRowCount = rowCounter;
}

ASqlTable::~ASqlTable()
{
}
