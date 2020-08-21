#pragma once
#include "ALayout.h"

class API_AUI_VIEWS AHorizontalLayout: public ALayout
{
private:
	int mSpacing = 0;

public:
	AHorizontalLayout()
	{
	}
	AHorizontalLayout(int spacing): mSpacing(spacing)
	{
	}

	void onResize(int x, int y, int width, int height) override;

	int getMinimumWidth() override;
	int getMinimumHeight() override;

	void setSpacing(int spacing) override;
};
