#pragma once
#include "AUI/Common/AException.h"
#include <AUI/Data.h>

class API_AUI_DATA SQLException: public AException
{
public:
	SQLException();
	explicit SQLException(const AString& message);
	virtual ~SQLException();
};
