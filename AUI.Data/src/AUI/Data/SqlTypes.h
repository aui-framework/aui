#pragma once

#include "AUI/Common/AString.h"

enum SqlType
{
	ST_STRING
};

struct SqlColumn
{
	AString name;
	SqlType type;
};