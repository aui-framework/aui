#pragma once

#include <cstdint>

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
    inline bool operator<(const AModelIndex& other) const {
	    return hash() < other.hash();
	}

	[[nodiscard]] inline uint64_t hash() const {
	    uint64_t hash = uint32_t(mRow);
	    hash <<= 32u;
	    hash |= uint32_t(mColumn);
	    return hash;
	}
};
