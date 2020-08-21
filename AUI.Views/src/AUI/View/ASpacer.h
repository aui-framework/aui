#pragma once
#include "AView.h"

class API_AUI_VIEWS ASpacer: public AView
{
public:
	ASpacer(int w = 4, int h = 4)
	{
		setExpanding({ w, h });
	}
	virtual ~ASpacer() = default;

	
};
