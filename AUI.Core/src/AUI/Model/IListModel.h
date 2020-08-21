#pragma once
#include "AUI/Common/AVariant.h"

class API_AUI_CORE IListModel
{
public:
	virtual ~IListModel() = default;

	virtual size_t listSize() = 0;
	virtual AVariant listItemAt(size_t index) = 0;
};
