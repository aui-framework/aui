#pragma once
#include <cstdint>
#include "AUI/Common/AByteBuffer.h"
#include "AUI/Common/AVariant.h"

class ISqlDriverRow
{
public:
	virtual ~ISqlDriverRow() = default;

	virtual AVariant getValue(size_t index) = 0;
};
