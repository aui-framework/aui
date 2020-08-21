#pragma once
#include "AUI/Model/ASimpleTable.h"
#include <AUI/Data.h>

#include "ASqlQueryResult.h"


class API_AUI_DATA ASqlTable: public ASimpleTable
{
public:
	ASqlTable(_<ASqlQueryResult> result);
	~ASqlTable() override;
};
