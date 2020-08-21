#pragma once
#include "ALayout.h"

class API_AUI_VIEWS AStackedLayout: public ALayout
{
public:
	AStackedLayout()
	{
	}
	virtual ~AStackedLayout() = default;

	void onResize(int x, int y, int width, int height) override;
	int getMinimumWidth() override;
	int getMinimumHeight() override;
};
