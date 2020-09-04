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

	AModelIndex(int row)
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

	inline bool operator==(const AModelIndex& other) const {
	    return mRow == other.mRow && mColumn == other.mColumn;
	}
	inline bool operator!=(const AModelIndex& other) const {
	    return mRow != other.mRow || mColumn != other.mColumn;
	}

};
