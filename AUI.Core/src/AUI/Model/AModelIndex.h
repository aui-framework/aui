#pragma once

class AModelIndex
{
private:
	int mRow;
	int mColumn;
	
public:
	AModelIndex(int row, int column)
		: mRow(row),
		  mColumn(column)
	{
	}

	explicit AModelIndex(int row)
		: mRow(row)
	{
	}

	AModelIndex() = default;

	int getRow() const
	{
		return mRow;
	}

	int getColumn() const
	{
		return mColumn;
	}
};
